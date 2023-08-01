//------------------------------------------ Includes ----------------------------------------------

#include "isd4000App.h"
#include "islSdkFirmware.h"
#include "platform/maths.h"
#include "platform/debug.h"

using namespace IslSdk;

//--------------------------------------------------------------------------------------------------
Isd4000App::Isd4000App(void)
{
	m_isd4000 = nullptr;
	printLog("------------\n d    Set settings to defualt\n s    Save settings to file\n u    Update firmware\n------------\n");
}
//--------------------------------------------------------------------------------------------------
Isd4000App::~Isd4000App(void)
{
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::disconnectAllEvents(void)
{
	m_isd4000->onError.disconnect(slotError);
	m_isd4000->onDelete.disconnect(slotDelete);
	m_isd4000->onConnect.disconnect(slotConnect);
	m_isd4000->onDisconnect.disconnect(slotDisconnect);
	m_isd4000->onPortAdded.disconnect(slotPortAdded);
	m_isd4000->onPortChanged.disconnect(slotPortChanged);
	m_isd4000->onPortRemoved.disconnect(slotPortRemoved);
	m_isd4000->onPortSelected.disconnect(slotPortSelected);
	m_isd4000->onInfoChanged.disconnect(slotDeviceInfo);
	m_isd4000->onPacketStats.disconnect(slotPacketStats);
	m_isd4000->onBootloader.disconnect(slotBootloader);

	m_isd4000->ahrs.onData.disconnect(slotAhrsData);
	m_isd4000->gyro.onData.disconnect(slotGyroData);
	m_isd4000->accel.onData.disconnect(slotAccelData);
	m_isd4000->mag.onData.disconnect(slotMagData);
	m_isd4000->accel.onCalProgress.disconnect(slotAccelCal);

	m_isd4000->onPressure.disconnect(slotPressure);
	m_isd4000->onTemperature.disconnect(slotTemperature);
	m_isd4000->onScriptDataReceived.disconnect(slotScriptDataReceived);
	m_isd4000->onSettingsUpdated.disconnect(slotSettingsUpdated);
	m_isd4000->onPressureCalCert.disconnect(slotPressureCalCert);
	m_isd4000->onTemperatureCalCert.disconnect(slotTemperatureCalCert);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::setDevice(Isd4000* isd4000)
{
	if (m_isd4000 != nullptr)
	{
		disconnectAllEvents();
	}

	m_isd4000 = isd4000;

	if (m_isd4000 != nullptr)
	{
		m_isd4000->onError.connect(slotError);
		m_isd4000->onDelete.connect(slotDelete);
		m_isd4000->onConnect.connect(slotConnect);
		m_isd4000->onDisconnect.connect(slotDisconnect);
		m_isd4000->onPortAdded.connect(slotPortAdded);
		m_isd4000->onPortChanged.connect(slotPortChanged);
		m_isd4000->onPortRemoved.connect(slotPortRemoved);
		m_isd4000->onPortSelected.connect(slotPortSelected);
		m_isd4000->onInfoChanged.connect(slotDeviceInfo);
		m_isd4000->onPacketStats.connect(slotPacketStats);
		m_isd4000->onBootloader.connect(slotBootloader);

		isd4000SensorRates_t rates;
		rates.pressure = 100;
		rates.orientation = 100;
		rates.gyro = 100;
		rates.accel = 100;
		rates.mag = 100;
		rates.temperature = 200;
		m_isd4000->setSensorRates(rates);

		//m_isd4000->ahrs.onData.connect(slotAhrsData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_isd4000->gyro.onData.connect(slotGyroData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_isd4000->accel.onData.connect(slotAccelData);			// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_isd4000->mag.onData.connect(slotMagData);				
		m_isd4000->onPressure.connect(slotPressure);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		m_isd4000->onTemperature.connect(slotTemperature);			// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		m_isd4000->onScriptDataReceived.connect(slotScriptDataReceived);
		m_isd4000->onSettingsUpdated.connect(slotSettingsUpdated);
		m_isd4000->onPressureCalCert.connect(slotPressureCalCert);
		m_isd4000->onTemperatureCalCert.connect(slotTemperatureCalCert);
	}
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::doTask(int32_t key, const str_t* path)
{
	std::string filePath(path);

	switch (key)
	{
	case 'd':
		isd4000Settings_t settings;
		m_isd4000->getDefaultSettings(settings);
		m_isd4000->setSettings(settings, true);
		break;

	case 's':
		filePath += "settings.xml";
		m_isd4000->saveConfig(path);
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
		islSdkDeviceProgramFirmware(m_isd4000->id, fwId);
		break;

	default:
		break;
	}
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackError(Device& device, uint32_t errorCode, const utf8_t* msg)
{
	printLog("callbackError. %s\n", msg);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackDeleteted(Device& device)
{
	printLog("callbackDeleteted %04u-%04u\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackConnect(Device& device)
{
	printLog("ISD4000 %04u-%04u connected and ready\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackDisconnect(Device& device)
{
	printLog("ISD4000 %04u-%04u disconnected\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackPortAdded(Device& device, SysPort& port, const connectionMeta_t& meta)
{
	printLog("Port %s added to %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackPortChanged(Device& device, SysPort& port, const connectionMeta_t& meta)
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
void Isd4000App::callbackPortRemoved(Device& device, SysPort& port)
{
	printLog("Port %s removed from %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackPortSelected(Device& device, SysPort& port, const connectionMeta_t& meta)
{
	printLog("Port %s selected for %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackDeviceInfo(Device& device, const deviceInfo_t& info)
{
	printLog("callbackDeviceInfo\n");
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackPacketStats(Device& device, const deviceStats_t& stats)
{
	printLog("Packet stats for %04u-%04u Tx:%u, Rx:%u\n", device.info.pn, device.info.sn, stats.txPacketCount, stats.rxPacketCount);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackBootloader(Device& device, const bootloaderProgress_t& progress)
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
void Isd4000App::callbackAhrs(Ahrs& ahrs, uint64_t timeUs, const quaternion_t& q, real_t magHeading, real_t turnsCount)
{
	eulerAngles_t euler;

	quaternionToEulerAngles(&q, 0, &euler);
	printLog("H:%.1f    P:%.2f    R%.2f\n", radToDeg(euler.heading), radToDeg(euler.pitch), radToDeg(euler.roll));
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackGyroData(GyroSensor& gyro, const vector3_t& v)
{
	printLog("Gyro x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackAccelData(AccelSensor& accel, const vector3_t& v)
{
	printLog("Accel x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackMagData(MagSensor& mag, const vector3_t& v)
{
	printLog("Mag x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackAccelCal(AccelSensor& accel, uint32_t axis, const vector3_t& v, uint8_t progress)
{
	printLog("accel %u   %.2f, %.2f, %.2f,   0x%02x\n", axis, v.x, v.y, v.z, (uint32_t)progress);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackPressureData(Isd4000& isd4000, uint64_t timeUs, real_t pressureBar, real_t depthM, real_t pressureBarRaw)
{
	printLog("Pressure %.5f Bar, Depth %.3f Meters\n", pressureBar, depthM);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackTemperatureData(Isd4000& isd4000, real_t temperatureC, real_t temperatureRawC)
{
	printLog("Temperature %.2fC\n", temperatureC);
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackScriptDataReceived(Isd4000& isa500)
{
	printLog("Script data received\n");
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackSettingsUpdated(Isd4000& isa500, bool_t ok)
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
void Isd4000App::callbackPressureCal(Isd4000& isd4000, const isd4000PressureCal_t& cal)
{
	printLog("Pressure cal received\n");
}
//--------------------------------------------------------------------------------------------------
void Isd4000App::callbackTemperatureCal(Isd4000& isd4000, const isd4000TemperatureCal_t& cal)
{
	printLog("Temperature cal received\n");
}
//--------------------------------------------------------------------------------------------------