//------------------------------------------ Includes ----------------------------------------------

#include "device.h"
#include "sdkInterface.h"
#include "platform/debug.h"
#include <vector>

using namespace IslSdk;

//------------------------------------------- Globals ----------------------------------------------

static std::vector<Device*> deviceList;

//--------------------------------------------------------------------------------------------------
const std::vector<Device*>& Device::getList()
{
	return deviceList;
}
//--------------------------------------------------------------------------------------------------
Device* Device::findById(uint32_t id)
{
	Device* device;

	device = nullptr;
	for (size_t i = 0; i < deviceList.size(); i++)
	{
		if (deviceList[i]->m_id == id)
		{
			device = deviceList[i];
			break;
		}
	}

	return device;
}
//--------------------------------------------------------------------------------------------------
void Device::deleteAll()
{
	while (deviceList.size())
	{
		delete deviceList[0];
	}
}
//--------------------------------------------------------------------------------------------------
void Device::removeLoggerInst(const Logger& logger)
{
	for (size_t i = 0; i < deviceList.size(); i++)
	{
		if (deviceList[i]->m_log.logger == &logger)
		{
			deviceList[i]->setLogger(nullptr, 0);
		}
	}
}
//--------------------------------------------------------------------------------------------------
Device::Device(uint32_t id, const deviceInfo_t& info)
{
	m_id = id;
	m_reconnectCount = 0;
	m_info = info;
	m_isConnected = false;
	m_connection = nullptr;
	m_log.logger = nullptr;
	m_log.trackId = 0;
	m_log.active = false;
	
	deviceList.push_back(this);
}
//--------------------------------------------------------------------------------------------------
Device::~Device()
{
	islSdkDeviceDelete(m_id);

	onDelete(*this);

	for (size_t i = 0; i < deviceList.size(); i++)
	{
		if (deviceList[i] == this)
		{
			deviceList.erase(deviceList.begin() + i);
			break;
		}
	}
}
//--------------------------------------------------------------------------------------------------
void Device::handleBaseEvent(uint64_t timeMs, uint32_t eventId, const void* data)
{
	portConnectionInfo_t* portConnectionInfo;
	deviceConnection_t* connection;
	SysPort* sysPort;

	switch (eventId)
	{
	case EVENT_DEVICE_ERROR:
		onError(*this, ((sdkError_t*)data)->errorCode, ((sdkError_t*)data)->msg);
		break;

	case EVENT_DEVICE_CONNECTED:
	case EVENT_DEVICE_DISCONNECTED:
		connectionEvent(eventId == EVENT_DEVICE_CONNECTED);
		break;

	case EVENT_DEVICE_INFO:
		m_info = *((deviceInfo_t*)data);
		onInfoChanged(*this, m_info);
		break;

	case EVENT_DEVICE_DELETED:
		break;

	case EVENT_DEVICE_CONNECTION_ADDED:
		portConnectionInfo = (portConnectionInfo_t*)data;
		sysPort = SysPort::findById(portConnectionInfo->id);
		if (sysPort != nullptr)
		{
			m_connectionList.emplace_back(sysPort, portConnectionInfo->meta);
			onPortAdded(*this, *sysPort, portConnectionInfo->meta);
		}
		break;

	case EVENT_DEVICE_CONNECTION_CHANGED:
		portConnectionInfo = (portConnectionInfo_t*)data;
		connection = findDeviceConnection(portConnectionInfo->id);
		if (connection != nullptr)
		{
			connection->meta = portConnectionInfo->meta;
			onPortChanged(*this, *connection->sysPort, connection->meta);
		}
		break;

	case EVENT_DEVICE_CONNECTION_REMOVED:
		for (uint32_t i = 0; i < m_connectionList.size(); i++)
		{
			if (m_connectionList[i].sysPort->id == *((uint32_t*)data))
			{
				sysPort = m_connectionList[i].sysPort;
				m_connectionList.erase(m_connectionList.begin() + i);
				onPortRemoved(*this, *sysPort);
				break;
			}
		}
		break;

	case EVENT_DEVICE_CONNECTION_SELECTED:
		portConnectionInfo = (portConnectionInfo_t*)data;
		m_connection = findDeviceConnection(portConnectionInfo->id);
		if (m_connection != nullptr)
		{
			onPortSelected(*this, *m_connection->sysPort, m_connection->meta);
		}
		break;

	case EVENT_DEVICE_STATS:
		onPacketStats(*this, *((deviceStats_t*)data));
		break;

	case EVENT_DEVICE_BOOTLOADER_PROGRESS:
		onBootloader(*this, *((bootloaderProgress_t*)data));
		break;

	default:
		break;
	}
}
//---------------------------------------------------------------------------------------------------
void Device::connect()
{
	if (!m_isConnected)
	{
		islSdkDeviceConnect(m_id);
	}
}
//---------------------------------------------------------------------------------------------------
void Device::reset()
{
	islSdkDeviceReset(m_id);
}
//---------------------------------------------------------------------------------------------------
void Device::setConnection(const SysPort& port)
{
	islSdkDeviceSetConnection(m_id, port.id);
}
//---------------------------------------------------------------------------------------------------
void Device::uploadFirmware(uint32_t fwId)
{
	islSdkDeviceProgramFirmware(m_id, fwId);
}
//---------------------------------------------------------------------------------------------------
void Device::cancelFirmwareUpload()
{
	islSdkDeviceCancelFirmware(m_id);
}
//---------------------------------------------------------------------------------------------------
void Device::connectionEvent(bool_t connected)
{
	m_isConnected = connected;

	if (connected)
	{
		onConnect(*this);
	}
	else
	{
		onDisconnect(*this);
	}
	m_reconnectCount += connected;
}
//---------------------------------------------------------------------------------------------------
void Device::setLogger(Logger* logger, uint32_t appId)
{
	if (logger != nullptr)
	{
		if (m_log.logger != logger)
		{
			if (m_log.logger != nullptr)
			{
				m_log.logger->onNewFile.disconnect(slotOnNewLogFile);
			}
			m_log.logger = logger;
			m_log.trackId = logger->addTrack(appId, (m_info.pn << 16) | m_info.sn, m_info.pid);
			m_log.logger->onNewFile.connect(slotOnNewLogFile);
		}
	}
	else
	{
		m_log.logger = nullptr;
		m_log.trackId = 0;
	}

	if (m_log.active)
	{
		m_log.active = false;
		startStopLogging(false);
	}
}
//---------------------------------------------------------------------------------------------------
void Device::startStopLogging(bool_t start)
{
	if (m_log.active != start && m_log.logger != nullptr)
	{
		m_log.active = start;
	}
}
//---------------------------------------------------------------------------------------------------
deviceConnection_t* Device::findDeviceConnection(uint32_t sysPortId)
{
	uint32_t i;

	for (i = 0; i < m_connectionList.size(); i++)
	{
		if (m_connectionList[i].sysPort->id == sysPortId)
		{
			return &m_connectionList[i];
		}
	}
	return nullptr;
}
//---------------------------------------------------------------------------------------------------
void Device::callbackNewLogFile(Logger& logger, const utf8_t* filename)
{
	if (m_log.active)
	{
		startStopLogging(true);
	}
}
//---------------------------------------------------------------------------------------------------
