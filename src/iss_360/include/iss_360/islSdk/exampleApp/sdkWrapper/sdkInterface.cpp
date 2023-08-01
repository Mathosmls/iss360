//------------------------------------------ Includes ----------------------------------------------

#include "sdkInterface.h"
#include "platformSetup.h"
#include "iss360.h"
#include "isa500.h"
#include "isd4000.h"
#include "ism3d.h"

using namespace IslSdk;

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

static void (*newPortCallback)(SysPort& port);
static void (*newDeviceCallback)(SysPort& port, Device& device);

//---------------------------------- Private Function Prototypes -----------------------------------

static void portEventHandler(uint64_t timeMs, uint32_t eventId, uint32_t id, const void* data);
static void deviceEventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data);
static void iss360EventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data);
static void isa500EventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data);
static void isd4000EventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data);
static void ism3dEventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data);
static void logEventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data);
static void createDevice(SysPort& inst, uint32_t deviceId, const deviceInfo_t& deviceInfo);

static Slot<SysPort&, uint32_t, const deviceInfo_t&> slotCreateDevice(&createDevice);

//--------------------------------------------------------------------------------------------------
uint16_t IslSdk::initialise(void(*newEvent)(void), void(*newPort)(SysPort& port), void(*newDevice)(SysPort& port, Device& device))
{
	platformSetup();
	newPortCallback = newPort;
	newDeviceCallback = newDevice;

	if (islSdkInitialise(newEvent, &portEventHandler, &deviceEventHandler))
	{
		islSdkLogSetEvent(&logEventHandler);
		islSdkIss360SetEvent(&iss360EventHandler);
		islSdkIsa500SetEvent(&isa500EventHandler);
		islSdkIsd4000SetEvent(&isd4000EventHandler);
		islSdkIsm3dSetEvent(&ism3dEventHandler);
		return islSdkVersion();
	}
	return 0;
}
//--------------------------------------------------------------------------------------------------
void IslSdk::cleanUp()
{
	Device::deleteAll();
	SysPort::deleteAll();
	Logger::deleteAll();
}
//--------------------------------------------------------------------------------------------------
void IslSdk::setDeviceRediscoveryTimings(uint32_t count, uint32_t timeoutMs)
{
	islSdkSetDeviceRediscoveryTimings(count, timeoutMs);
}
//--------------------------------------------------------------------------------------------------
static void portEventHandler(uint64_t timeMs, uint32_t eventId, uint32_t id, const void* data)
{
	SysPort* sysPort;

	sysPort = SysPort::findById(id);

	if (sysPort != nullptr)
	{
		sysPort->handleEvent(timeMs, eventId, data);

		if (eventId == EVENT_PORT_DELETED)
		{
			delete sysPort;
		}
	}
	else if (eventId == EVENT_PORT_CREATED)
	{
		sysPort = new SysPort(id, *((portInfo_t*)data));

		if (sysPort != nullptr)
		{
			sysPort->onDeviceDiscovered.connect(slotCreateDevice);
			if (newPortCallback != nullptr)
			{
				newPortCallback(*sysPort);
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------
static void logEventHandler(uint64_t timeMs, uint32_t eventId, uint32_t id, const void* data)
{
	Logger* logger;

	if (eventId == EVENT_LOG_CREATED)
	{
		logger = Logger::findByFilename((str_t*)data, id);
	}
	else
	{
		logger = Logger::findById(id);
	}

	if (logger != nullptr)
	{
		logger->handleEvent(timeMs, eventId, data);
	}
}
//--------------------------------------------------------------------------------------------------
static void deviceEventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data)
{
	Device* device;

	device = Device::findById(id);

	if (device != nullptr)
	{
		device->handleBaseEvent(timeMs, eventType, data);

		if (eventType == EVENT_DEVICE_DELETED)
		{
			delete device;
		}
	}
}
//--------------------------------------------------------------------------------------------------
static void iss360EventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data)
{
	Iss360* device;

	device = (Iss360*)Device::findById(id);

	if (device != nullptr)
	{
		device->handleEvent(timeMs, eventType, data);
	}
}
//--------------------------------------------------------------------------------------------------
static void isa500EventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data)
{
	Isa500* device;

	device = (Isa500*)Device::findById(id);

	if (device != nullptr)
	{
		device->handleEvent(timeMs, eventType, data);
	}
}
//--------------------------------------------------------------------------------------------------
static void isd4000EventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data)
{
	Isd4000* device;

	device = (Isd4000*)Device::findById(id);

	if (device != nullptr)
	{
		device->handleEvent(timeMs, eventType, data);
	}
}
//--------------------------------------------------------------------------------------------------
static void ism3dEventHandler(uint64_t timeMs, uint32_t eventType, uint32_t id, const void* data)
{
	Ism3d* device;

	device = (Ism3d*)Device::findById(id);

	if (device != nullptr)
	{
		device->handleEvent(timeMs, eventType, data);
	}
}
//--------------------------------------------------------------------------------------------------
static void createDevice(SysPort& port, uint32_t deviceId, const deviceInfo_t& deviceInfo)
{
	Device* device = Device::findById(deviceId);

	if (device == nullptr)
	{
		switch (deviceInfo.pid)
		{
		case PID_ISA500:
			device = new Isa500(deviceId, deviceInfo);
			break;

		case PID_ISD4000:
			device = new Isd4000(deviceId, deviceInfo);
			break;

		case PID_ISM3D:
			device = new Ism3d(deviceId, deviceInfo);
			break;

		case PID_ISS360:
			device = new Iss360(deviceId, deviceInfo);
			break;
		case PID_UNKNOWN:
		default:
			device = new Device(deviceId, deviceInfo);
			break;
		}

		if (device != nullptr && newDeviceCallback != nullptr)
		{
			newDeviceCallback(port, *device);
		}
	}
	else
	{
		if (device->reconnectCount && !device->info.inUse && !device->isConnected)
		{
			device->connect();
		}
	}
}
//--------------------------------------------------------------------------------------------------
