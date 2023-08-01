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
	#include <pthread.h>				// add -pthread option to complier and linker to resolve undefined reference
	#include <time.h>
	#include <unistd.h>
#endif


using namespace IslSdk;

//--------------------------------------- Public Constants -----------------------------------------

#define SDK_THREAD_SLEEP_INTERVAL_MS	40
#define IP_TO_UINT(a, b, c, d)	(a & 0xff) | ((b << 8) & 0xff00)  | ((c << 16) & 0xff0000)  | ((d << 24) & 0xff000000)

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

static volatile bool_t sdkThreadRun;
Iss360App* iss360App = nullptr;
Isa500App* isa500App = nullptr;
Isd4000App* isd4000App = nullptr;
Ism3dApp* ism3dApp = nullptr;
static utf8_t appPath[256];

//---------------------------------- Private Function Prototypes -----------------------------------

static void* sdkThread(void* param);
static void printMsg(const utf8_t* msg, uint32_t length);
static void newSdkEvent(void);
static void newPort(SysPort& inst);
static void newDevice(SysPort& port, Device& device);

static void portOpen(SysPort& inst, bool_t failed);
static void portClosed(SysPort& inst);
static void portDiscoveryStarted(SysPort& inst, autoDiscoveryType_t type);
static void portDiscoveryEvent(SysPort& inst, const autoDiscoveryInfo_t& info);
static void portDiscoveryFinished(SysPort& inst, const autoDiscoveryInfo_t& info);
static void portData(SysPort& inst, const uint8_t* data, uint32_t size);

Slot<SysPort&, bool_t> slotPortOpen(&portOpen);
Slot<SysPort&> slotPortClosed(&portClosed);
Slot<SysPort&, autoDiscoveryType_t> slotPortDiscoveryStarted(&portDiscoveryStarted);
Slot<SysPort&, const autoDiscoveryInfo_t&> slotPortDiscoveryEvent(&portDiscoveryEvent);
Slot<SysPort&, const autoDiscoveryInfo_t&> slotPortDiscoveryFinished(&portDiscoveryFinished);
Slot<SysPort&, const uint8_t*, uint32_t> slotPortData(&portData);

//--------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
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
	
	while (1)
	{
	#ifdef OS_WINDOWS
		Sleep(40);
	#else
		usleep(40);
	#endif
		
		islSdkFireEvents();

		if (keyboardPressed())
		{
			int32_t key = getKey();

			if (iss360App != nullptr)
			{
				iss360App->doTask(key, &appPath[0]);
			}

			if (isa500App != nullptr)
			{
				isa500App->doTask(key, &appPath[0]);
			}

			if (isd4000App != nullptr)
			{
				isd4000App->doTask(key, &appPath[0]);
			}

			if (ism3dApp != nullptr)
			{
				ism3dApp->doTask(key, &appPath[0]);
			}

			if (key == 'x')
			{
				break;
			}
		}
	}

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
static void* sdkThread(void* param)
{
	while (sdkThreadRun)
	{
		islSdkRun();													// Run all SDK tasks

	#ifdef OS_WINDOWS
		Sleep(SDK_THREAD_SLEEP_INTERVAL_MS);							// Yeild thread execution 
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
static void printMsg(const utf8_t* msg, uint32_t length)
{
	printf("%s", (char*)msg);
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
static void newPort(SysPort& inst)
{
	inst.onOpen.connect(slotPortOpen);
	inst.onClose.connect(slotPortClosed);
	inst.onDiscoveryStarted.connect(slotPortDiscoveryStarted);
	inst.onDiscoveryEvent.connect(slotPortDiscoveryEvent);
	inst.onDiscoveryFinished.connect(slotPortDiscoveryFinished);
	//inst.onData.connect(slotPortData);

	if (inst.type == PORT_NETWORK)
	{	
		uint32_t ipAddress = IP_TO_UINT(255, 255, 255, 255);
		inst.startDiscovery(0, 0, PID_ANY, ipAddress, 33005, 1000);
	}
	else if (inst.type == PORT_SERIAL || inst.type == PORT_SOL)
	{
		inst.startDiscovery(0, 0, PID_ANY, nullptr, 0, 250);
	}
}
//--------------------------------------------------------------------------------------------------
static void newDevice(SysPort& port, Device& device)
{
	switch (device.info.pid)
	{
	case PID_ISS360:
		printLog("Found ISS360 sonar %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
		iss360App = new Iss360App();
		iss360App->setDevice((Iss360*)&device);
		break;

	case PID_ISA500:
		printLog("Found ISA500 altimeter %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
		isa500App = new Isa500App();
		isa500App->setDevice((Isa500*)&device);
		break;

	case PID_ISD4000:
		printLog("Found ISD4000 %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
		isd4000App = new Isd4000App();
		isd4000App->setDevice((Isd4000*)&device);
		break;

	case PID_ISM3D:
		printLog("Found ISM3D %04u-%04u on port %s\n", device.info.pn, device.info.sn, port.name.c_str());
		ism3dApp = new Ism3dApp();
		ism3dApp->setDevice((Ism3d*)&device);
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
static void portOpen(SysPort& inst, bool_t failed)
{
	if (!failed)
	{
		printLog("%s Open\n", inst.name.c_str());
	}
}
//--------------------------------------------------------------------------------------------------
static void portClosed(SysPort& inst)
{
	printLog("%s closed\n", inst.name.c_str());
}
//--------------------------------------------------------------------------------------------------
static void portDiscoveryStarted(SysPort& inst, autoDiscoveryType_t type)
{
	printLog("%s Discovery Started\n", inst.name.c_str());
}
//--------------------------------------------------------------------------------------------------
static void portDiscoveryEvent(SysPort& inst, const autoDiscoveryInfo_t& info)
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
static void portDiscoveryFinished(SysPort& inst, const autoDiscoveryInfo_t& info)
{
	printLog("%s Discovery Finished\n", inst.name.c_str());
}
//--------------------------------------------------------------------------------------------------
static void portData(SysPort& inst, const uint8_t* data, uint32_t size)
{
	printLog("Port data, size: %u\n", size);
}
//--------------------------------------------------------------------------------------------------