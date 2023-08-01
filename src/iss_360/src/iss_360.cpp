//------------------------------------------ Includes ----------------------------------------------

#include "sdkInterface.h"
#include "platformSetup.h"
#include "platform/debug.h"
#include <stdio.h>
#include "iss360App.h"
#include "isa500App.h"
#include "isd4000App.h"
#include "ism3dApp.h"
#include "terminal.h"

#ifdef OS_WINDOWS
#include "windows.h"
#else
#include <pthread.h> // add -pthread option to complier and linker to resolve undefined reference
#include <time.h>
#include <unistd.h>
#endif

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sonar_ping_msgs/msg/sonar_ping.hpp"
#include "image_transport/image_transport.hpp"

using namespace IslSdk;
using namespace std::chrono_literals;

//--------------------------------------- Public Constants -----------------------------------------

#define SDK_THREAD_SLEEP_INTERVAL_MS 40
#define IP_TO_UINT(a, b, c, d) (a & 0xff) | ((b << 8) & 0xff00) | ((c << 16) & 0xff0000) | ((d << 24) & 0xff000000)

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

static volatile bool_t sdkThreadRun;
Iss360App *iss360App = nullptr;
Isa500App *isa500App = nullptr;
Isd4000App *isd4000App = nullptr;
Ism3dApp *ism3dApp = nullptr;
static utf8_t appPath[256];

//---------------------------------- Private Function Prototypes -----------------------------------

static void *sdkThread(void *param);
static void printMsg(const utf8_t *msg, uint32_t length);
static void newSdkEvent(void);
static void newPort(SysPort &inst);
static void newDevice(SysPort &port, Device &device);

static void portOpen(SysPort &inst, bool_t failed);
static void portClosed(SysPort &inst);
static void portDiscoveryStarted(SysPort &inst, autoDiscoveryType_t type);
static void portDiscoveryEvent(SysPort &inst, const autoDiscoveryInfo_t &info);
static void portDiscoveryFinished(SysPort &inst, const autoDiscoveryInfo_t &info);
static void portData(SysPort &inst, const uint8_t *data, uint32_t size);
// void callbackPingData(Iss360 &inst, const iss360Ping_t &data);

Slot<SysPort &, bool_t> slotPortOpen(&portOpen);
Slot<SysPort &> slotPortClosed(&portClosed);
Slot<SysPort &, autoDiscoveryType_t> slotPortDiscoveryStarted(&portDiscoveryStarted);
Slot<SysPort &, const autoDiscoveryInfo_t &> slotPortDiscoveryEvent(&portDiscoveryEvent);
Slot<SysPort &, const autoDiscoveryInfo_t &> slotPortDiscoveryFinished(&portDiscoveryFinished);
Slot<SysPort &, const uint8_t *, uint32_t> slotPortData(&portData);

//--------------------------------------------------------------------------------------------------
class iss360_pub : public rclcpp::Node
{
public:
    iss360_pub()
        : Node("iss_360_pub"), count_(0)
    {

        auto publish_image_desc = rcl_interfaces::msg::ParameterDescriptor{};
        auto publish_scan_desc = rcl_interfaces::msg::ParameterDescriptor{};
        auto publish_ping_desc = rcl_interfaces::msg::ParameterDescriptor{};
        auto threshold_desc = rcl_interfaces::msg::ParameterDescriptor{};

        publish_image_desc.description = "If = 'true', publish image produced by the sonar";
        publish_scan_desc.description = "If = 'true', publish the sonar scan = the distance where an obstacle is found for all the possible angles ";
        publish_ping_desc.description = "If = 'true', publish all the intensities values for one sonar ping/echo (for one angle) ";
        publish_ping_desc.description = "Intensity value that we consider as an obstacle = this parameter * max intensity";

        this->declare_parameter("publish_image", true, publish_image_desc);
        this->declare_parameter("publish_scan", true, publish_scan_desc);
        this->declare_parameter("publish_ping", true, publish_ping_desc);
        this->declare_parameter("threshold_obstacle", 0.6, threshold_desc);

        publish_image = this->get_parameter("publish_image").as_bool();
        publish_scan = this->get_parameter("publish_scan").as_bool();
        publish_ping = this->get_parameter("publish_ping").as_bool();
        threshold = this->get_parameter("threshold_obstacle").as_double();

        if (publish_scan)
            publisher_scan = this->create_publisher<sensor_msgs::msg::LaserScan>("iss_360_scan", 10);
        if (publish_ping)
            publisher_ping = this->create_publisher<sonar_ping_msgs::msg::SonarPing>("iss_360_ping", 10);
        if (publish_image)
        {
            image_transport::ImageTransport it(node);
            pub = it.advertise("iss_360_image", 1);
        }

        timer_data = this->create_wall_timer(
            2ms, std::bind(&iss360_pub::read_data, this));
    }
    Slot<Iss360 &, const iss360Ping_t &> slotPingData2{this, &iss360_pub::callbackPingData};

private:
    void read_data()
    {
        islSdkFireEvents();
        if (keyboardPressed())
        {
            int32_t key = getKey();

            if (iss360App != nullptr)
            {
                iss360App->doTask(key, &appPath[0]);
            }

            if (key == 'x')
            {
                resetTerminalMode();
                rclcpp::shutdown();
            }
        }

        if ((iss360App != nullptr && iss360App->get_iss360() != nullptr && !connect))
        {

            iss360App->get_iss360()->onPingData.connect(iss360_pub::slotPingData2);
            connect = true;
        }
    };

    void callbackPingData(Iss360 &inst, const iss360Ping_t &data)
    {
        auto iss360_data = iss360App->get_iss360();
        if (iss360_data->last_ping.dataCount != UINT32_MAX)
        {
            // ------------------ INIT AND SETTINGS ------------------------
            if (iss360_data->new_setting && (publish_scan || publish_ping))
            {
                printLog("INIT \n\n");
                settings_iss360 = iss360_data->settings.setup;
                sonar_scan.angle_min = (float)islSdkIss360ConvertHeadAngleToDeg(settings_iss360.sectorStart) * M_PI / 180;
                sonar_scan.angle_max = (float)islSdkIss360ConvertHeadAngleToDeg((settings_iss360.sectorStart + settings_iss360.sectorSize) % 12800) * M_PI / 180;
                sonar_scan.angle_increment = (float)islSdkIss360ConvertHeadAngleToDeg(settings_iss360.stepSize) * M_PI / 180.0;
                sonar_scan.range_min = (float)settings_iss360.minRangeMm * 0.001;
                sonar_scan.range_max = (float)settings_iss360.maxRangeMm * 0.001;
                settings_iss360.stepSize = abs(settings_iss360.stepSize);
                sonar_ping.gain = settings_iss360.digitalGain;
                sonar_ping.speed_of_sound = settings_iss360.speedOfSound;
                sonar_ping.intensities.resize((int)settings_iss360.imageDataPoint);

                if (settings_iss360.sectorSize == 0)
                {
                    nb_angle = std::ceil(12800 / settings_iss360.stepSize) + 1;
                    settings_iss360.sectorSize = 12800;
                }
                else
                {
                    nb_angle = std::ceil((float)settings_iss360.sectorSize / (float)settings_iss360.stepSize) + 1;
                }
                printLog("nb_angle  %d ", nb_angle);
                sonar_scan.ranges.resize(nb_angle);
                sonar_scan.intensities.resize(nb_angle);
                iss360_data->new_setting = false;
                sonar_scan.time_increment = -1;
                old_time_scan = this->now().seconds();
            }

            // ------------------ ROS MESSAGES, LASER SCAN AND CUSTOM MESSAGE ------------------------
            if (iss360_data->last_ping.angle != last_angle)
            {
                
                auto now = this->now();
                last_angle = iss360_data->last_ping.angle;

                sonar_ping.header.stamp = now;
                sonar_ping.intensities.assign(iss360_data->last_ping.data, iss360_data->last_ping.data + iss360_data->last_ping.dataCount);
                int i_obstacle = sonar_ping.intensities.begin() - std::upper_bound(sonar_ping.intensities.begin(), sonar_ping.intensities.end(), (threshold * pow(256, (2 - settings_iss360.data8Bit))));
                sonar_ping.range = settings_iss360.minRangeMm + ((settings_iss360.maxRangeMm - settings_iss360.minRangeMm) / ((float)settings_iss360.imageDataPoint - 1.0) * i_obstacle);

                if (publish_scan)
                {
                    printLog("   !!  here 1  !!  ");

                    if (!read_duration)
                    {
                        sonar_scan.header.stamp = now;
                        old_time_increment = now.seconds();
                        read_duration = true;
                    }
                    else if (sonar_scan.time_increment == -1)
                    {
                        sonar_scan.time_increment = now.seconds() - old_time_increment;
                    }

                    int start_angle_diff = iss360_data->last_ping.angle - settings_iss360.sectorStart;
                    angle_ran = static_cast<int>(std::abs(start_angle_diff)) % 12800;
                    angle_ran = ((int)iss360_data->last_ping.stepSize <= 0) ? 12800 - angle_ran : angle_ran;
                    index = angle_ran / settings_iss360.stepSize;

                    if (index <= nb_angle - 1 && index >= 0)
                    {

                        sonar_scan.ranges[index] = sonar_ping.range;
                        sonar_scan.intensities[index] = sonar_ping.intensities[i_obstacle];
                         printLog("   !!  here  !!  ");

                        if ((std::abs(angle_ran) < settings_iss360.stepSize) ||
                            (angle_ran >= static_cast<int>(settings_iss360.sectorSize) && !settings_iss360.flybackMode) && publish_scan)
                        {
                            if (!dont_publish_next)
                            {

                                sonar_scan.scan_time = now.seconds() - old_time_scan;
                                publisher_scan->publish(sonar_scan);
                                printLog("   !!   PUBLISH  !!  ");
                                sonar_scan.time_increment = -1;
                                read_duration = false;
                                old_time_scan = now.seconds();
                            }
                            else
                            {
                                dont_publish_next = false;
                                printLog("   !!  NOT PUBLISHED  !!  ");
                            }
                        }
                    }
                    else
                    {
                        dont_publish_next = true; // The sonar can come back one step before the start.
                    }
                }
                if (publish_ping)
                    publisher_ping->publish(sonar_ping);

                // ------------------ PUBLISH THE IMAGE CREATED BY THE SONAR ------------------------
                if (publish_image)
                {
                    if (publish2fast == 0)
                    {
                        auto sonar_img = reinterpret_cast<uint32_t *>(iss360App->m_sonarImage.buf.mem);
                        uint32_t step = (iss360App->m_sonarImage.buf.width * 24) * 3;
                        uint32_t imageSize = step * iss360App->m_sonarImage.buf.height;
                        // uint8_t *data = (uint8_t *)std::malloc(imageSize);
                        uint8_t *ptr;
                        std::vector<uint8_t> data(imageSize);

                        // // I think this is actually not the place to do that, because it implies to recreate the image each time and because it's a gui job and not a ros problem
                        // auto center = iss360App->m_sonarImage.buf.width / 2;
                        // auto x1 = center;
                        // auto y1 = center;
                        // auto x2 = center + cos((float)islSdkIss360ConvertHeadAngleToDeg(iss360_data->last_ping.angle) * M_PI / 180 - M_PI_2) * center;
                        // auto y2 = center + sin((float)islSdkIss360ConvertHeadAngleToDeg(iss360_data->last_ping.angle) * M_PI / 180 - M_PI_2) * center;
                        // previous_angle = iss360_data->last_ping.angle;
                        // drawLineOnImage(sonar_img, iss360App->m_sonarImage.buf.width, iss360App->m_sonarImage.buf.height, x1, y1, x2, y2);

                        // Fill the image data
                        for (uint32_t row = 0; row < iss360App->m_sonarImage.buf.height; row++)
                        {
                            for (uint32_t x = 0; x < iss360App->m_sonarImage.buf.width; x++)
                            {
                                ptr = &data[step * (row) + x * 3];
                                *ptr++ = sonar_img[row * iss360App->m_sonarImage.buf.width + x] >> 0;
                                *ptr++ = sonar_img[row * iss360App->m_sonarImage.buf.width + x] >> 8;
                                *ptr++ = sonar_img[row * iss360App->m_sonarImage.buf.width + x] >> 16;
                            }
                        }

                        // Create and populate the image message
                        image.header.frame_id = "image_sonar_frame";
                        image.header.stamp = now;
                        image.height = iss360App->m_sonarImage.buf.height;
                        image.width = iss360App->m_sonarImage.buf.width;
                        image.encoding = "bgr8";
                        image.step = step;
                        image.is_bigendian = false;
                        image.data = std::move(data);

                        // Publish the image
                        pub.publish(image);
                    }

                    // If we set a very low stepsize, we send data very fast. Publish the image take to much time and we fill the queue of the event until a memory error
                    if (settings_iss360.stepSize < 200)
                    {
                        publish2fast++;
                    }
                    if (publish2fast > 4)
                    {
                        publish2fast = 0;
                    }
                }
            }
        }
    };

    // I think this is actually not the place to do that, because it implies to recreate the image each time and because it's a gui and not a ros problem
    // I let it there because it can be useful
    //  void drawLineOnImage(uint32_t *image_, uint32_t width, uint32_t height, int x1, int y1, int x2, int y2)
    // {
    //     int dx = abs(x2 - x1);
    //     int dy = abs(y2 - y1);
    //     int sx = (x1 < x2) ? 1 : -1;
    //     int sy = (y1 < y2) ? 1 : -1;
    //     int err = dx - dy;
    //     // printLog("x2 %d y2 %d \n", x2, y2);
    //     while (true)
    //     {
    //         if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height)
    //         {
    //             // Assuming the pixel format is 0x00RRGGBB (BGR8 format)
    //             image_[y1 * width + x1] = 0xFF0000;
    //         }
    //         if (x1 == x2 && y1 == y2)
    //         {
    //             break;
    //         }
    //         int e2 = 2 * err;
    //         if (e2 > -dy)
    //         {
    //             err -= dy;
    //             x1 += sx;
    //         }
    //         if (e2 < dx)
    //         {
    //             err += dx;
    //             y1 += sy;
    //         }
    //     }
    // }

    rclcpp::TimerBase::SharedPtr timer_data;
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr publisher_scan;
    rclcpp::Publisher<sonar_ping_msgs::msg::SonarPing>::SharedPtr publisher_ping;

    rclcpp::Node::SharedPtr node = rclcpp::Node::make_shared("image_transport_node");
    // image_transport::ImageTransport it(node);
    image_transport::Publisher pub;

    sensor_msgs::msg::LaserScan sonar_scan;
    sonar_ping_msgs::msg::SonarPing sonar_ping;

    bool first_angle_read = false;
    bool dont_publish_next = false;
    bool range_found = false;

    int angle_ran = 0;
    int index = 0;
    int nb_angle = 0;
    int threshold;
    uint16_t last_angle = 34464;

    double old_time_increment = 0.0;
    double old_time_scan = 0.0;
    bool read_duration = false;
    iss360SetupSettings_t settings_iss360;

    sensor_msgs::msg::Image image;

    size_t count_;

    bool publish_image;
    bool publish_scan;
    bool publish_ping;

    int publish2fast;
    bool connect = false;
};
//--------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    uint16_t versionBcd;

    getExePath(argv[0], &appPath[0], sizeof(appPath));

    islSdkDebugSetPrintFunc(&printMsg);

    versionBcd = IslSdk::initialise(newSdkEvent, newPort, newDevice);
    sdkThreadRun = TRUE;

    printLog("Impact Subsea SDK version V%u.%u.%u\n", (uint32_t)((versionBcd >> 8) & 0x0f), (uint32_t)((versionBcd >> 4) & 0x0f), (uint32_t)((versionBcd >> 0) & 0x0f));
    printLog("x -> exit\n\n");

#ifdef OS_WINDOWS
    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)sdkThread, NULL, 0, NULL);
#else
    pthread_t thread;
    pthread_create(&thread, nullptr, sdkThread, nullptr);
    setConioTerminalMode();
#endif

    // while (1)
    // {

    // }
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<iss360_pub>());
    rclcpp::shutdown();

    sdkThreadRun = FALSE;
    IslSdk::cleanUp();
#ifdef OS_WINDOWS
    WaitForSingleObject(thread, INFINITE);
#else
    pthread_join(thread, nullptr);
    resetTerminalMode();
#endif

    return 0;
}
//--------------------------------------------------------------------------------------------------
static void *sdkThread(void *param)
{
    while (sdkThreadRun)
    {
        islSdkRun(); // Run all SDK tasks

#ifdef OS_WINDOWS
        Sleep(SDK_THREAD_SLEEP_INTERVAL_MS); // Yeild thread execution
#else
        struct timespec ts;
        ts.tv_sec = SDK_THREAD_SLEEP_INTERVAL_MS / 1000;
        ts.tv_nsec = (SDK_THREAD_SLEEP_INTERVAL_MS % 1000) * 1000000;
        nanosleep(&ts, nullptr);
#endif
    }

    islSdkExit();

    return nullptr;
}
//--------------------------------------------------------------------------------------------------
static void printMsg(const utf8_t *msg, uint32_t length)
{
    printf("%s", (char *)msg);
}
//--------------------------------------------------------------------------------------------------
static void newSdkEvent(void)
{
    /*
    Now is the time to call islSdkFireEvents() to dispatch the SDK's event queue to the event handlers set by IslSdkInitialise, IslSdkIss360SetEvent,
    IslSdkLogSetEvent etc. If we do it here then the event handlers will be called from the SDK thread and you will likely need to handle cross thread
    syncing (unless you handle the data in the SDK thread which is not recommended!). One good approach is to post a message to the main thread and
    call islSdkFireEvents() from the main thread. Alternatively it might be just as efficient to call islSdkFireEvents() repetitively from the main
    thread. Note the SDK events are queued up with time stamps so calling IslSdkFireEvents at a low frequency might be suitable for your application.
    */
}
//--------------------------------------------------------------------------------------------------
static void newPort(SysPort &inst)
{
    inst.onOpen.connect(slotPortOpen);
    inst.onClose.connect(slotPortClosed);
    inst.onDiscoveryStarted.connect(slotPortDiscoveryStarted);
    inst.onDiscoveryEvent.connect(slotPortDiscoveryEvent);
    inst.onDiscoveryFinished.connect(slotPortDiscoveryFinished);
    // inst.onData.connect(slotPortData);

    if (inst.type == PORT_NETWORK)
    {
        uint32_t ipAddress = IP_TO_UINT(192, 168, 1, 200);
        inst.startDiscovery(0, 0, PID_ANY, ipAddress, 33005, 1000);
    }
    else if (inst.type == PORT_SERIAL || inst.type == PORT_SOL)
    {
        inst.startDiscovery(0, 0, PID_ANY, nullptr, 0, 250);
    }
}
//--------------------------------------------------------------------------------------------------
static void newDevice(SysPort &port, Device &device)
{
    switch (device.info.pid)
    {
    case PID_ISS360:
        printLog("Found ISS360 sonar %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
        iss360App = new Iss360App();
        iss360App->setDevice((Iss360 *)&device);
        break;

    case PID_ISA500:
        printLog("Found ISA500 altimeter %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
        isa500App = new Isa500App();
        isa500App->setDevice((Isa500 *)&device);
        break;

    case PID_ISD4000:
        printLog("Found ISD4000 %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
        isd4000App = new Isd4000App();
        isd4000App->setDevice((Isd4000 *)&device);
        break;

    case PID_ISM3D:
        printLog("Found ISM3D %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
        ism3dApp = new Ism3dApp();
        ism3dApp->setDevice((Ism3d *)&device);
        break;

    case PID_TESTJIG:
        printLog("Found Test Jig %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
        break;

    default:
        printLog("Found new device %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
        break;
    }

    if (!device.info.inUse && !device.isConnected)
    {
        device.connect();
    }
}
//--------------------------------------------------------------------------------------------------
static void portOpen(SysPort &inst, bool_t failed)
{
    if (!failed)
    {
        printLog("%s Open\n", inst.name.c_str());
    }
}
//--------------------------------------------------------------------------------------------------
static void portClosed(SysPort &inst)
{
    printLog("%s closed\n", inst.name.c_str());
}
//--------------------------------------------------------------------------------------------------
static void portDiscoveryStarted(SysPort &inst, autoDiscoveryType_t type)
{
    printLog("%s Discovery Started\n", inst.name.c_str());
}
//--------------------------------------------------------------------------------------------------
static void portDiscoveryEvent(SysPort &inst, const autoDiscoveryInfo_t &info)
{
#define UINT_TO_IP (ip)

    if (info.port.type == PORT_NETWORK)
    {
        uint32_t ip = info.port.meta.ipAddress;
        printLog("%s Discovering at IP %u.%u.%u.%u:%u\n", inst.name.c_str(), ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff, info.port.meta.port);
    }
    else
    {
        printLog("%s Discovering at baudrate %u \n", inst.name.c_str(), info.port.meta.baudrate);
    }
}
//--------------------------------------------------------------------------------------------------
static void portDiscoveryFinished(SysPort &inst, const autoDiscoveryInfo_t &info)
{
    printLog("%s Discovery Finished\n", inst.name.c_str());
}
//--------------------------------------------------------------------------------------------------
static void portData(SysPort &inst, const uint8_t *data, uint32_t size)
{
    printLog("Port data, size: %u\n", size);
}
//--------------------------------------------------------------------------------------------------
