//------------------------------------------ Includes ----------------------------------------------

#include "ism3dApp.h"
#include "islSdkFirmware.h"
#include "platform/maths.h"
#include "platform/debug.h"

using namespace IslSdk;

//--------------------------------------------------------------------------------------------------
Ism3dApp::Ism3dApp(void)
{
	m_ism3d = nullptr;
	printLog("------------\n d    Set settings to defualt\n s    Save settings to file\n u    Update firmware\n------------\n");
}
//--------------------------------------------------------------------------------------------------
Ism3dApp::~Ism3dApp(void)
{

}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::disconnectAllEvents(void)
{
	m_ism3d->onError.disconnect(slotError);
	m_ism3d->onDelete.disconnect(slotDelete);
	m_ism3d->onConnect.disconnect(slotConnect);
	m_ism3d->onDisconnect.disconnect(slotDisconnect);
	m_ism3d->onPortAdded.disconnect(slotPortAdded);
	m_ism3d->onPortChanged.disconnect(slotPortChanged);
	m_ism3d->onPortRemoved.disconnect(slotPortRemoved);
	m_ism3d->onPortSelected.disconnect(slotPortSelected);
	m_ism3d->onInfoChanged.disconnect(slotDeviceInfo);
	m_ism3d->onPacketStats.disconnect(slotPacketStats);
	m_ism3d->onBootloader.disconnect(slotBootloader);

	m_ism3d->ahrs.onData.disconnect(slotAhrsData);
	m_ism3d->gyro.onData.disconnect(slotGyroData);
	m_ism3d->gyroBk.onData.disconnect(slotGyroData);
	m_ism3d->accel.onData.disconnect(slotAccelData);
	m_ism3d->accelBk.onData.disconnect(slotAccelData);
	m_ism3d->mag.onData.disconnect(slotMagData);
	m_ism3d->accel.onCalProgress.disconnect(slotAccelCal);

	m_ism3d->onScriptDataReceived.disconnect(slotScriptDataReceived);
	m_ism3d->onSettingsUpdated.disconnect(slotSettingsUpdated);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::setDevice(Ism3d* ism3d)
{
	if (m_ism3d != nullptr)
	{
		disconnectAllEvents();
	}

	m_ism3d = ism3d;
	
	if (m_ism3d != nullptr)
	{
		m_ism3d->onError.connect(slotError);
		m_ism3d->onDelete.connect(slotDelete);
		m_ism3d->onConnect.connect(slotConnect);
		m_ism3d->onDisconnect.connect(slotDisconnect);
		m_ism3d->onPortAdded.connect(slotPortAdded);
		m_ism3d->onPortChanged.connect(slotPortChanged);
		m_ism3d->onPortRemoved.connect(slotPortRemoved);
		m_ism3d->onPortSelected.connect(slotPortSelected);
		m_ism3d->onInfoChanged.connect(slotDeviceInfo);
		m_ism3d->onPacketStats.connect(slotPacketStats);
		m_ism3d->onBootloader.connect(slotBootloader);

		ism3dSensorRates_t rates;
		rates.orientation = 100;
		rates.gyro = 100;
		rates.accel = 100;
		rates.mag = 100;
		m_ism3d->setSensorRates(rates);

		m_ism3d->ahrs.onData.connect(slotAhrsData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_ism3d->gyro.onData.connect(slotGyroData);			// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		m_ism3d->gyroBk.onData.connect(slotGyroData);			// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_ism3d->accel.onData.connect(slotAccelData);			// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		m_ism3d->accelBk.onData.connect(slotAccelData);			// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		//m_ism3d->mag.onData.connect(slotMagData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		m_ism3d->accel.onCalProgress.connect(slotAccelCal);
		
		m_ism3d->onScriptDataReceived.connect(slotScriptDataReceived);
		m_ism3d->onSettingsUpdated.connect(slotSettingsUpdated);
	}
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::doTask(int32_t key, const str_t* path)
{
	std::string filePath(path);

	switch (key)
	{
	case 'd':
		ism3dSettings_t settings;
		m_ism3d->getDefaultSettings(settings);
		m_ism3d->setSettings(settings, true);
		break;

	case 's':
		filePath += "settings.xml";
		m_ism3d->saveConfig(filePath.c_str());
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
		islSdkDeviceProgramFirmware(m_ism3d->id, fwId);
		break;

	default:
		break;
	}
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackError(Device& device, uint32_t errorCode, const utf8_t* msg)
{
	printLog("callbackError. %s\n", msg);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackDeleteted(Device& device)
{
	printLog("callbackDeleteted %04u-%04u\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackConnect(Device& device)
{
	printLog("ISM3D %04u-%04u connected and ready\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackDisconnect(Device& device)
{
	printLog("ISM3D %04u-%04u disconnected\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackPortAdded(Device& device, SysPort& port, const connectionMeta_t& meta)
{
	printLog("Port %s added to %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackPortChanged(Device& device, SysPort& port, const connectionMeta_t& meta)
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
void Ism3dApp::callbackPortRemoved(Device& device, SysPort& port)
{
	printLog("Port %s removed from %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackPortSelected(Device& device, SysPort& port, const connectionMeta_t& meta)
{
	printLog("Port %s selected for %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackDeviceInfo(Device& device, const deviceInfo_t& info)
{
	printLog("callbackDeviceInfo\n");
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackPacketStats(Device& device, const deviceStats_t& stats)
{
	printLog("Packet stats for %04u-%04u Tx:%u, Rx:%u\n", device.info.pn, device.info.sn, stats.txPacketCount, stats.rxPacketCount);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackBootloader(Device& device, const bootloaderProgress_t& progress)
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
void Ism3dApp::callbackAhrs(Ahrs& ahrs, uint64_t timeUs, const quaternion_t& q, real_t magHeading, real_t turnsCount)
{
	eulerAngles_t euler;

	quaternionToEulerAngles(&q, 0, &euler);
	printLog("H:%.1f    P:%.2f    R%.2f\n", radToDeg(euler.heading), radToDeg(euler.pitch), radToDeg(euler.roll));
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackGyroData(GyroSensor& gyro, const vector3_t& v)
{
	printLog("Gyro %u, x:%.2f, y:%.2f, z:%.2f\n", gyro.sensorNumber, v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackAccelData(AccelSensor& accel, const vector3_t& v)
{
	printLog("Accel %u, x:%.2f, y:%.2f, z:%.2f\n", accel.sensorNumber, v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackMagData(MagSensor& mag, const vector3_t& v)
{
	printLog("Mag x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackAccelCal(AccelSensor& accel, uint32_t axis, const vector3_t& v, uint8_t progress)
{
	printLog("accel %u   %.2f, %.2f, %.2f,   0x%02x\n", axis, v.x, v.y, v.z, (uint32_t)progress);
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackScriptDataReceived(Ism3d& ism3d)
{
	printLog("Script data received\n");
}
//--------------------------------------------------------------------------------------------------
void Ism3dApp::callbackSettingsUpdated(Ism3d& ism3d, bool_t ok)
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