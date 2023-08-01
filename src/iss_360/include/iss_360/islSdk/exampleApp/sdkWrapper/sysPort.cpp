//------------------------------------------ Includes ----------------------------------------------

#include "sysPort.h"
#include <vector>

using namespace IslSdk;

//------------------------------------------- Globals ----------------------------------------------

static std::vector<SysPort*> sysPortList;

//--------------------------------------------------------------------------------------------------
const std::vector<SysPort*>& SysPort::getList()
{
	return sysPortList;
}
//--------------------------------------------------------------------------------------------------
SysPort* SysPort::findById(uint32_t id)
{
	SysPort* sysPort;

	sysPort = nullptr;
	for (size_t i = 0; i < sysPortList.size(); i++)
	{
		if (sysPortList[i]->m_id == id)
		{
			sysPort = sysPortList[i];
			break;
		}
	}

	return sysPort;
}
//--------------------------------------------------------------------------------------------------
void SysPort::deleteAll()
{
	while (sysPortList.size())
	{
		delete sysPortList[0];
	}
}
//--------------------------------------------------------------------------------------------------
void SysPort::createSol(bool_t useTcp, bool_t useTelnetRfc2217, uint32_t ipAddress, uint16_t port)
{
	islSdkPortSolCreate(useTcp, useTelnetRfc2217, ipAddress, port);
}
//--------------------------------------------------------------------------------------------------
SysPort::SysPort(uint32_t id, portInfo_t& info)
{
	m_id = id;
	m_name.assign((char*)info.name);
	m_type = info.type;
	m_isOpen = false;
	m_isAutoDiscovering = false;
	sysPortList.push_back(this);
}
//--------------------------------------------------------------------------------------------------
SysPort::~SysPort()
{
	onDelete(*this);

	if (m_type == PORT_SOL)
	{
		islSdkPortSolDelete(m_id);
	}

	for (size_t i = 0; i < sysPortList.size(); i++)
	{
		if (sysPortList[i] == this)
		{
			sysPortList.erase(sysPortList.begin() + i);
			break;
		}
	}
}
//--------------------------------------------------------------------------------------------------
void SysPort::signalSubscribersChanged(uint32_t subscriberCount)
{
	if (subscriberCount <= 1)
	{
		islSdkPortDataEventEnable(m_id, subscriberCount == 1);
	}
}
//--------------------------------------------------------------------------------------------------
void SysPort::handleEvent(uint64_t timeMs, uint32_t eventId, const void* data)
{
	autoDiscoveryType_t* autoDiscoveryType;
	autoDiscoveryInfo_t* autoDiscoveryInfo;
	portDeviceDiscovery_t* portDeviceDiscovery;
	portNmeaString_t* nmeaStr;
	portData_t* portData;

	switch (eventId)
	{
	case EVENT_PORT_ERROR:
		onError(*this, ((sdkError_t*)data)->errorCode, ((sdkError_t*)data)->msg);
		break;

	case EVENT_PORT_CREATED:
		break;

	case EVENT_PORT_DELETED:
		break;

	case EVENT_PORT_OPEN:
	case EVENT_PORT_OPEN_FAILED:
		m_isOpen = eventId == EVENT_PORT_OPEN;
		onOpen(*this, !m_isOpen);
		break;

	case EVENT_PORT_CLOSE:
		m_isOpen = false;
		onClose(*this);
		break;

	case EVENT_PORT_STATS:
		onPortStats(*this, *((portStats_t*)data));
		break;

	case EVENT_PORT_AUTO_DISCOVERY_STARTED:
		autoDiscoveryType = (autoDiscoveryType_t*)data;
		m_isAutoDiscovering = true;
		onDiscoveryStarted(*this, *autoDiscoveryType);
		break;

	case EVENT_PORT_AUTO_DISCOVERY_EVENT:
		autoDiscoveryInfo = (autoDiscoveryInfo_t*)data;
		onDiscoveryEvent(*this, *autoDiscoveryInfo);
		break;

	case EVENT_PORT_AUTO_DISCOVERY_FINISHED:
		autoDiscoveryInfo = (autoDiscoveryInfo_t*)data;
		m_isAutoDiscovering = false;
		onDiscoveryFinished(*this, *autoDiscoveryInfo);
		break;

	case EVENT_PORT_DEVICE_DISCOVERED:
		portDeviceDiscovery = (portDeviceDiscovery_t*)data;
		onDeviceDiscovered(*this, portDeviceDiscovery->deviceId, portDeviceDiscovery->info);
		break;

	case EVENT_PORT_NMEA_STRING:
		nmeaStr = (portNmeaString_t*)data;
		onNmeaString(*this, nmeaStr->str, nmeaStr->size, nmeaStr->checkSumOk);
		break;

	case EVENT_PORT_DATA:
		portData = (portData_t*)data;
		onData(*this, portData->data, portData->size);
		break;
	}
}
//---------------------------------------------------------------------------------------------------
void SysPort::stopDiscovery()
{
	islSdkPortAutoDiscoverStop(m_id);
}
//---------------------------------------------------------------------------------------------------
void SysPort::startDiscovery(uint32_t pn, uint32_t sn, devicePid_t pid, const uint32_t* baudrateList, uint32_t baudCount, uint32_t timeoutMs)
{
	static const uint32_t defaultBaudrates[] = { 115200, 9600, 57600, 38400, 19200 };
	const uint32_t* baudrate;
	uint32_t i;
	
	if (baudrateList == nullptr || baudCount == 0)
	{
		baudrate = &defaultBaudrates[0];
		baudCount = countof(defaultBaudrates);
	}
	else
	{
		baudrate = baudrateList;
	}

	for (i = 0; i < baudCount; i++)
	{
		islSdkPortSerialAutoDiscoverDevices(m_id, pn, sn, pid, *baudrate++, timeoutMs);
	}
}
//---------------------------------------------------------------------------------------------------
void SysPort::startDiscovery(uint32_t pn, uint32_t sn, devicePid_t pid, uint32_t ipAddress, uint16_t port, uint32_t timeoutMs)
{
	islSdkPortLanAutoDiscoverDevices(pn, sn, pid, ipAddress, port, timeoutMs);
}
//---------------------------------------------------------------------------------------------------
void SysPort::startNmeaDiscovery(const uint32_t* baudrateList, uint32_t baudCount, uint32_t timeoutMs)
{
	static const uint32_t defaultBaudrates[] = { 115200, 9600, 57600, 38400, 19200 };
	const uint32_t* baudrate;
	uint32_t i;
	
	if (baudrateList == nullptr || baudCount == 0)
	{
		baudrate = &defaultBaudrates[0];
		baudCount = countof(defaultBaudrates);
	}
	else
	{
		baudrate = baudrateList;
	}

	for (i = 0; i < baudCount; i++)
	{
		islSdkPortSerialAutoDiscoverNmeaDevices(m_id, *baudrate++, timeoutMs);
	}
}
//---------------------------------------------------------------------------------------------------
void SysPort::open()
{
	islSdkPortOpen(m_id);
}
//---------------------------------------------------------------------------------------------------
void SysPort::close()
{
	islSdkPortClose(m_id);
}
//---------------------------------------------------------------------------------------------------
void SysPort::write(uint8_t* data, uint32_t size, connectionMeta_t& meta)
{
	islSdkPortWrite(m_id, data, size, &meta);
}
//---------------------------------------------------------------------------------------------------
void SysPort::config(uint32_t baudrate, uint8_t dataBits, uartParity_t parity, uartStopBits_t stopBits)
{
	islSdkPortConfigureSerial(m_id, baudrate, dataBits, parity, stopBits);
}
//---------------------------------------------------------------------------------------------------
