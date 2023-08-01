//------------------------------------------ Includes ----------------------------------------------

#include "isa500App.h"
#include "islSdkFirmware.h"
#include "platform/maths.h"
#include "platform/debug.h"

using namespace IslSdk;

//--------------------------------------------------------------------------------------------------
Isa500App::Isa500App(void)
{
	m_isa500 = nullptr;

	printLog("------------\n d    Set settings to defualt\n s    Save settings to file\n u    Update firmware\n p    Ping now\n------------\n");
}
//--------------------------------------------------------------------------------------------------
Isa500App::~Isa500App(void)
{
}
//--------------------------------------------------------------------------------------------------
void Isa500App::disconnectAllEvents(void)
{
	m_isa500->onError.disconnect(slotError);
	m_isa500->onDelete.disconnect(slotDelete);
	m_isa500->onConnect.disconnect(slotConnect);
	m_isa500->onDisconnect.disconnect(slotDisconnect);
	m_isa500->onPortAdded.disconnect(slotPortAdded);
	m_isa500->onPortChanged.disconnect(slotPortChanged);
	m_isa500->onPortRemoved.disconnect(slotPortRemoved);
	m_isa500->onPortSelected.disconnect(slotPortSelected);
	m_isa500->onInfoChanged.disconnect(slotDeviceInfo);
	m_isa500->onPacketStats.disconnect(slotPacketStats);
	m_isa500->onBootloader.disconnect(slotBootloader);

	m_isa500->ahrs.onData.disconnect(slotAhrsData);
	m_isa500->gyro.onData.disconnect(slotGyroData);
	m_isa500->accel.onData.disconnect(slotAccelData);
	m_isa500->mag.onData.disconnect(slotMagData);
	m_isa500->accel.onCalProgress.disconnect(slotAccelCal);

	m_isa500->onEcho.disconnect(slotEchoData);
	m_isa500->onTemperature.disconnect(slotTemperatureData);
	m_isa500->onVoltage.disconnect(slotVoltageData);
	m_isa500->onTrigger.disconnect(slotTriggerData);
	m_isa500->onScriptDataReceived.disconnect(slotScriptDataReceived);
	m_isa500->onSettingsUpdated.disconnect(slotSettingsUpdated);
	m_isa500->onPingTest.disconnect(slotPingTestData);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::setDevice(Isa500* isa500)
{
	if (m_isa500 != nullptr)
	{
		disconnectAllEvents();
	}

	m_isa500 = isa500;

	if (m_isa500 != nullptr)
	{
		m_isa500->onError.connect(slotError);
		m_isa500->onDelete.connect(slotDelete);
		m_isa500->onConnect.connect(slotConnect);
		m_isa500->onDisconnect.connect(slotDisconnect);
		m_isa500->onPortAdded.connect(slotPortAdded);
		m_isa500->onPortChanged.connect(slotPortChanged);
		m_isa500->onPortRemoved.connect(slotPortRemoved);
		m_isa500->onPortSelected.connect(slotPortSelected);
		m_isa500->onInfoChanged.connect(slotDeviceInfo);
		m_isa500->onPacketStats.connect(slotPacketStats);
		m_isa500->onBootloader.connect(slotBootloader);

		isa500SensorRates_t rates;
		rates.ping = 0;
		rates.orientation = 100;
		rates.gyro = 100;
		rates.accel = 100;
		rates.mag = 100;
		rates.temperature = 1000;
		rates.voltage = 1000;
		m_isa500->setSensorRates(rates);

		//m_isa500->ahrs.onData.connect(slotAhrsData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_isa500->gyro.onData.connect(slotGyroData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_isa500->accel.onData.connect(slotAccelData);			// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_isa500->mag.onData.connect(slotMagData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		m_isa500->accel.onCalProgress.connect(slotAccelCal);
		m_isa500->onEcho.connect(slotEchoData);						// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_isa500->onTemperature.connect(slotTemperatureData);		// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_isa500->onVoltage.connect(slotVoltageData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		m_isa500->onTrigger.connect(slotTriggerData);
		m_isa500->onScriptDataReceived.connect(slotScriptDataReceived);
		m_isa500->onSettingsUpdated.connect(slotSettingsUpdated);
		m_isa500->onPingTest.connect(slotPingTestData);
	}
}
//--------------------------------------------------------------------------------------------------
void Isa500App::doTask(int32_t key, const str_t* path)
{
	std::string filePath(path);

	switch (key)
	{
	case 'd':
		isa500Settings_t settings;
		m_isa500->getDefaultSettings(settings);
		m_isa500->setSettings(settings, true);
		break;

	case 's':
		filePath += "settings.xml";
		m_isa500->saveConfig(filePath.c_str());
		break;

	case 'u':
		uint32_t fwId;
		firmwareInfo_t fwInfo;
		filePath += "firmware.fwi";
		fwId = islSdkFirmwareLoadFile(filePath.c_str());
		if (islSdkFirmwareGetInfo(fwId, &fwInfo))
		{
			printLog("\r\nFirmware file info: ID (0x%08x)   Size %u bytes", fwInfo.id, fwInfo.size);
			printLog("    V%hu.%hu.%hu\n", (fwInfo.versionBCD >> 8) & 0xf, (fwInfo.versionBCD >> 4) & 0xf, fwInfo.versionBCD & 0xf);
		}
		islSdkDeviceProgramFirmware(m_isa500->id, fwId);
		break;

	case 'p':
		m_isa500->pingNow();
		break;

	default:
		break;
	}
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackError(Device& device, uint32_t errorCode, const utf8_t* msg)
{
	printLog("callbackError. %s\n", msg);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackDeleteted(Device& device)
{
	printLog("callbackDeleteted %04u-%04u\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackConnect(Device& device)
{
	printLog("ISA500 %04u-%04u connected and ready\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackDisconnect(Device& device)
{
	printLog("ISA500 %04u-%04u disconnected\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackPortAdded(Device& device, SysPort& port, const connectionMeta_t& meta)
{
	printLog("Port %s added to %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackPortChanged(Device& device, SysPort& port, const connectionMeta_t& meta)
{
	if (port.type == PORT_SERIAL || port.type == PORT_SOL)
	{
		printLog("Baudrate for %04u-%04u on port %s changed to %u\n", device.info.pn, device.info.sn, port.name.c_str(), meta.baudrate);
	}
	else if (port.type == PORT_NETWORK)
	{
		printLog("IP address for %04u-%04u on port %s changed to %u.%u.%u.%u\n", device.info.pn, device.info.sn, port.name.c_str(), meta.ipAddress & 0xff, (meta.ipAddress >> 8) & 0xff, (meta.ipAddress >> 16) & 0xff, (meta.ipAddress >> 24) & 0xff);
	}
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackPortRemoved(Device& device, SysPort& port)
{
	printLog("Port %s removed from %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackPortSelected(Device& device, SysPort& port, const connectionMeta_t& meta)
{
	printLog("Port %s selected for %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackDeviceInfo(Device& device, const deviceInfo_t& info)
{
	printLog("callbackDeviceInfo\n");
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackPacketStats(Device& device, const deviceStats_t& stats)
{
	printLog("Packet stats for %04u-%04u Tx:%u, Rx:%u\n", device.info.pn, device.info.sn, stats.txPacketCount, stats.rxPacketCount);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackBootloader(Device& device, const bootloaderProgress_t& progress)
{
	switch (progress.status)
	{
	case bootloaderStatus_t::BOOTLDR_INVOKING:
		printLog("Invoking bootloader\n");
		break;

	case bootloaderStatus_t::BOOTLDR_UPLOADING:
		printLog("Uploading firmware %u bytes of %u\n", progress.loadedSize, progress.totalSize);
		break;

	case bootloaderStatus_t::BOOTLDR_BOOTING_APP:
		printLog("Firmware upload complete\n");
		break;

	case bootloaderStatus_t::BOOTLDR_ERR_WRONG_PID:
	case bootloaderStatus_t::BOOTLDR_ERR_BUSY:
	case bootloaderStatus_t::BOOTLDR_ERR_FAILED:
	case bootloaderStatus_t::BOOTLDR_ERR_CANCELED:
		printLog("Firmware upload error\n");
		break;
	}
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackAhrs(Ahrs& ahrs, uint64_t timeUs, const quaternion_t& q, real_t magHeading, real_t turnsCount)
{
	eulerAngles_t euler;

	quaternionToEulerAngles(&q, 0, &euler);
	printLog("H:%.1f    P:%.2f    R%.2f\n", radToDeg(euler.heading), radToDeg(euler.pitch), radToDeg(euler.roll));
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackGyroData(GyroSensor& gyro, const vector3_t& v)
{
	printLog("Gyro x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackAccelData(AccelSensor& accel, const vector3_t& v)
{
	printLog("Accel x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackMagData(MagSensor& mag, const vector3_t& v)
{
	printLog("Mag x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackAccelCal(AccelSensor& accel, uint32_t axis, const vector3_t& v, uint8_t progress)
{
	printLog("accel %u   %.2f, %.2f, %.2f,   0x%02x\n", axis, v.x, v.y, v.z, (uint32_t)progress);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackEchoData(Isa500& isa500, const isa500Echoes_t& echoes)
{
	if (echoes.length)
	{
		// echoes.length defines the length of echoes.echo array and is limited to isa500.settings.multiEchoLimit
		// echoes.totalEchoCount is the number of received echoes and has nothing to do with the length of echoes.echo array
		printLog("Echo received, range %.3f meters. Total Echoes: %u\n", echoes.echo[echoes.selectedIdx].totalTof * isa500.settings.speedOfSound * 0.5, echoes.totalEchoCount);
	}
	else
	{
		printLog("No echoes received\n");
	}
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackTemperatureData(Isa500& isa500, real_t temperatureC)
{
	printLog("Temperature %.2f\n", temperatureC);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackVoltageData(Isa500& isa500, real_t voltage12)
{
	printLog("Voltage %.2fV\n", voltage12);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackTriggerData(Isa500& isa500, bool_t risingEdge)
{
	printLog("Trigger, edge:%u\n", (uint32_t)risingEdge);
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackScriptDataReceived(Isa500& isa500)
{
	printLog("Script data received\n");
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackSettingsUpdated(Isa500& isa500, bool_t ok)
{
	if (ok)
	{
		printLog("Settings updated ok\n");
	}
	else
	{
		printLog("Settings failed to update\n");
	}
}
//--------------------------------------------------------------------------------------------------
void Isa500App::callbackPingTestData(Isa500& isa500, const isa500TestWaveform_t& data)
{
	printLog("Test ping data\n");
}
//--------------------------------------------------------------------------------------------------
