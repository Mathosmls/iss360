//------------------------------------------ Includes ----------------------------------------------

#include "iss360App.h"
#include "islSdkFirmware.h"
#include "platform/maths.h"
#include "platform/debug.h"
#include "platform/file.h"
#include "platform/mem.h"
#include <filesystem>


using namespace IslSdk;
using std::filesystem::current_path;

static bool_t bmpFileSave(const str_t *fileName, const uint32_t *image, uint16_t bpp, uint32_t width, uint32_t height);

//--------------------------------------------------------------------------------------------------
Iss360App::Iss360App(void)
{
	m_iss360 = nullptr;
	printLog("------------\n d    Set settings to defualt\n s    Save settings to file\n u    Update firmware\n p    Start scanning\n P    Stop scanning\n------------\n");
	m_pingCount = 0;
	m_palette.setToDefault();
	m_sonarImage.setBilinerInterpolation(true);
	m_sonarTexture.setBilinerInterpolation(false);
}
//--------------------------------------------------------------------------------------------------
Iss360App::~Iss360App(void)
{
}
//--------------------------------------------------------------------------------------------------
void Iss360App::disconnectAllEvents(void)
{
	m_iss360->onError.disconnect(slotError);
	m_iss360->onDelete.disconnect(slotDelete);
	m_iss360->onConnect.disconnect(slotConnect);
	m_iss360->onDisconnect.disconnect(slotDisconnect);
	m_iss360->onPortAdded.disconnect(slotPortAdded);
	m_iss360->onPortChanged.disconnect(slotPortChanged);
	m_iss360->onPortRemoved.disconnect(slotPortRemoved);
	m_iss360->onPortSelected.disconnect(slotPortSelected);
	m_iss360->onInfoChanged.disconnect(slotDeviceInfo);
	m_iss360->onPacketStats.disconnect(slotPacketStats);
	m_iss360->onBootloader.disconnect(slotBootloader);

	m_iss360->ahrs.onData.disconnect(slotAhrsData);
	m_iss360->gyro.onData.disconnect(slotGyroData);
	m_iss360->accel.onData.disconnect(slotAccelData);
	m_iss360->mag.onData.disconnect(slotMagData);
	m_iss360->accel.onCalProgress.disconnect(slotAccelCal);

	m_iss360->onSettingsUpdated.disconnect(slotSettingsUpdated);
	m_iss360->onPwrAndTemp.disconnect(slotPwrAndTemp);
	m_iss360->onPingData.disconnect(slotPingData);
	m_iss360->onAdcData.disconnect(slotAdcData);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::setDevice(Iss360 *iss360)
{
	if (m_iss360 != nullptr)
	{
		disconnectAllEvents();
	}

	m_iss360 = iss360;

	if (m_iss360 != nullptr)
	{
		m_iss360->onError.connect(slotError);
		m_iss360->onDelete.connect(slotDelete);
		m_iss360->onConnect.connect(slotConnect);
		m_iss360->onDisconnect.connect(slotDisconnect);
		m_iss360->onPortAdded.connect(slotPortAdded);
		m_iss360->onPortChanged.connect(slotPortChanged);
		m_iss360->onPortRemoved.connect(slotPortRemoved);
		m_iss360->onPortSelected.connect(slotPortSelected);
		m_iss360->onInfoChanged.connect(slotDeviceInfo);
		m_iss360->onPacketStats.connect(slotPacketStats);
		m_iss360->onBootloader.connect(slotBootloader);

		iss360SensorRates_t rates;
		rates.orientation = 100;
		rates.gyro = 100;
		rates.accel = 100;
		rates.mag = 100;
		rates.voltageAndTemp = 1000;
		m_iss360->setSensorRates(rates);
		m_iss360->last_ping.dataCount = UINT32_MAX;

		// m_iss360->ahrs.onData.connect(slotAhrsData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		// m_iss360->gyro.onData.connect(slotGyroData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		// m_iss360->accel.onData.connect(slotAccelData);			// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		// m_iss360->mag.onData.connect(slotMagData);				// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()

		m_iss360->onSettingsUpdated.connect(slotSettingsUpdated);
		// m_iss360->onPwrAndTemp.connect(slotPwrAndTemp);			// Subscribing to this event causes data to be sent from the device at the rate defined by setSensorRates()
		m_iss360->onPingData.connect(slotPingData);
		// m_iss360->onAdcData.connect(slotAdcData);					// Subscribing to this event causes raw ADC data to be sent
	}
}
//--------------------------------------------------------------------------------------------------
void Iss360App::doTask(int32_t key, const str_t *path)
{
	std::string filePath(path);

	switch (key)
	{
	case 'd':
		iss360Settings_t settings;
		m_iss360->getDefaultSettings(settings);
		m_iss360->setSystemSettings(settings.system, TRUE);
		m_iss360->setAcousticSettings(settings.acoustic, TRUE);
		m_iss360->setSetupSettings(settings.setup, TRUE);
		break;

	case 's':
		filePath += "settings.xml";
		m_iss360->saveConfig(filePath.c_str());
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
		islSdkDeviceProgramFirmware(m_iss360->id, fwId);
		break;

	case 'p':
		m_iss360->startScanning();
		break;

	case 'P':
		m_iss360->stopScanning();
		break;

	case 'i':
		if (m_sonarImage.buf.mem)
		{
			filePath += "sonar.bmp";
			bmpFileSave(filePath.c_str(), (uint32_t *)m_sonarImage.buf.mem, 24, m_sonarImage.buf.width, m_sonarImage.buf.height);
		}
		break;

	case 't':
		if (m_sonarTexture.buf.mem)
		{
			filePath += "sonar texture.bmp";
			bmpFileSave(filePath.c_str(), (uint32_t *)m_sonarTexture.buf.mem, 24, m_sonarTexture.buf.width, m_sonarTexture.buf.height);
		}
		break;

	case 'l':
	{
		auto path = current_path().string();
		path += "/src/sonar2/config/iss360_settings.xml";
		deviceInfo_t info_temp;
		iss360Settings_t settings_temp;
		iss360AhrsCal_t ahrsCal_temp;
		if (m_iss360->loadConfig(path.c_str(), &info_temp, &settings_temp, &ahrsCal_temp))
		{
			printLog("Setup and accoustic settings read from file\n");
			m_iss360->setAcousticSettings(settings_temp.acoustic, TRUE);
			m_iss360->setSetupSettings(settings_temp.setup, TRUE);
		}
		else
		{
			printLog("Error : not able to read the settings from file : %s\n", path.c_str());
		}
		break;
	}

	default:
		break;
	}
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackError(Device &device, uint32_t errorCode, const utf8_t *msg)
{
	printLog("callbackError. %s\n", msg);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackDeleteted(Device &device)
{
	printLog("callbackDeleteted %04u-%04u\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackConnect(Device &device)
{
	printLog("ISS360 %04u-%04u connected and ready\n", device.info.pn, device.info.sn);
	m_sonarImage.setBuffer(800, 800, TRUE);
	m_sonarImage.setSectorArea(0, m_iss360->settings.setup.maxRangeMm, m_iss360->settings.setup.sectorStart, m_iss360->settings.setup.sectorSize);

	// Optimal texture size to pass to the GPU - each pixel represents a data point. The GPU can then map this texture to circle (triangle fan)
	m_sonarTexture.setBuffer(m_iss360->settings.setup.imageDataPoint, 12800 / mathAbsInt(m_iss360->settings.setup.stepSize), TRUE);
	m_sonarTexture.setSectorArea(0, m_iss360->settings.setup.maxRangeMm, m_iss360->settings.setup.sectorStart, m_iss360->settings.setup.sectorSize);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackDisconnect(Device &device)
{
	printLog("ISS360 %04u-%04u disconnected\n", device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackPortAdded(Device &device, SysPort &port, const connectionMeta_t &meta)
{
	printLog("Port %s added to %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackPortChanged(Device &device, SysPort &port, const connectionMeta_t &meta)
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
void Iss360App::callbackPortRemoved(Device &device, SysPort &port)
{
	printLog("Port %s removed from %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackPortSelected(Device &device, SysPort &port, const connectionMeta_t &meta)
{
	printLog("Port %s selected for %04u-%04u\n", port.name.c_str(), device.info.pn, device.info.sn);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackDeviceInfo(Device &device, const deviceInfo_t &info)
{
	printLog("callbackDeviceInfo\n");
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackPacketStats(Device &device, const deviceStats_t &stats)
{
	printLog("Packet stats for %04u-%04u Tx:%u, Rx:%u\n", device.info.pn, device.info.sn, stats.txPacketCount, stats.rxPacketCount);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackBootloader(Device &device, const bootloaderProgress_t &progress)
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
void Iss360App::callbackAhrs(Ahrs &ahrs, uint64_t timeUs, const quaternion_t &q, real_t magHeading, real_t turnsCount)
{
	eulerAngles_t euler;

	quaternionToEulerAngles(&q, 0, &euler);
	printLog("H:%.1f    P:%.2f    R%.2f\n", radToDeg(euler.heading), radToDeg(euler.pitch), radToDeg(euler.roll));
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackGyroData(GyroSensor &gyro, const vector3_t &v)
{
	printLog("Gyro x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackAccelData(AccelSensor &accel, const vector3_t &v)
{
	printLog("Accel x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackMagData(MagSensor &mag, const vector3_t &v)
{
	printLog("Mag x:%.2f, y:%.2f, z:%.2f\n", v.x, v.y, v.z);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackAccelCal(AccelSensor &accel, uint32_t axis, const vector3_t &v, uint8_t progress)
{
	printLog("accel %u   %.2f, %.2f, %.2f,   0x%02x\n", axis, v.x, v.y, v.z, (uint32_t)progress);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackSettingsUpdated(Iss360 &iss360, bool_t ok, uint32_t set)
{
	if (ok)
	{
		printLog("Settings updated ok\n");
		m_sonarImage.setSectorArea(0, m_iss360->settings.setup.maxRangeMm, m_iss360->settings.setup.sectorStart, m_iss360->settings.setup.sectorSize);

		m_sonarTexture.setBuffer(m_iss360->settings.setup.imageDataPoint, 12800 / mathAbsInt(m_iss360->settings.setup.stepSize), TRUE);
		m_sonarTexture.setSectorArea(0, m_iss360->settings.setup.maxRangeMm, m_iss360->settings.setup.sectorStart, m_iss360->settings.setup.sectorSize);
	}
	else
	{
		printLog("Settings failed to update\n");
	}
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackPwrAndTemp(Iss360 &iss360, const cpuPwrTemp_t &data)
{
	printLog("CPU:%.1f, SYS:%.1f, 1.0V:%.2f, 1.8V:%.2f, 1.35V:%.2f\n", data.cpuTemperature, data.auxTemperature, data.core1V0, data.aux1V8, data.ddr1V35);
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackPingData(Iss360 &iss360, const iss360Ping_t &data)
{
	printLog("Ping data\n");
	m_iss360->image.render(m_sonarImage, &m_palette);
	m_iss360->image.renderTexture(m_sonarTexture, &m_palette);
	m_iss360->last_ping = data;
}
//--------------------------------------------------------------------------------------------------
void Iss360App::callbackAdcData(Iss360 &iss360, const iss360AdcPingData_t &data)
{
	printLog("adc data, size:%u\n", data.dataCount);
}
//--------------------------------------------------------------------------------------------------
static bool_t bmpFileSave(const str_t *fileName, const uint32_t *image, uint16_t bpp, uint32_t width, uint32_t height)
{
#define BITMAP_HEADER_SIZE 14
#define DIB_BITMAP_INFO_HEADER_SIZE 40
#define DIB_BITMAP_V4_HEADER_SIZE 108

	file_t *file;
	uint8_t header[BITMAP_HEADER_SIZE + DIB_BITMAP_V4_HEADER_SIZE];
	uint32_t rowWidth;
	uint32_t imageSize;
	uint32_t fileSize;
	uint32_t bfOffBits;
	uint32_t biSize;
	const uint16_t biPlanes = 1;
	uint8_t *data;
	uint8_t *ptr;
	uint32_t x;
	uint32_t row;

	debugAssert(fileName && image);

	rowWidth = ((width * bpp + 31) / 32) * 4;
	imageSize = rowWidth * height;

	if (bpp == 32)
	{
		fileSize = BITMAP_HEADER_SIZE + DIB_BITMAP_V4_HEADER_SIZE + imageSize;
		bfOffBits = BITMAP_HEADER_SIZE + DIB_BITMAP_V4_HEADER_SIZE;
		biSize = DIB_BITMAP_V4_HEADER_SIZE;
	}
	else
	{
		fileSize = BITMAP_HEADER_SIZE + DIB_BITMAP_INFO_HEADER_SIZE + imageSize;
		bfOffBits = BITMAP_HEADER_SIZE + DIB_BITMAP_INFO_HEADER_SIZE;
		biSize = DIB_BITMAP_INFO_HEADER_SIZE;
	}

	memSet(&header[0], 0, sizeof(header));
	header[0] = 'B';
	header[1] = 'M';
	memCopy(&header[2], &fileSize, 4);
	memCopy(&header[10], &bfOffBits, 4);

	memCopy(&header[14], &biSize, 4);
	memCopy(&header[18], &width, 4);
	memCopy(&header[22], &height, 4);
	memCopy(&header[26], &biPlanes, 2);
	memCopy(&header[28], &bpp, 2);
	memCopy(&header[34], &imageSize, 4);

	if (bpp == 32)
	{
		header[30] = 3;
		header[56] = 0xff;
		header[59] = 0xff;
		header[62] = 0xff;
		header[69] = 0xff;

		header[70] = 0x20;
		header[71] = 0x6e;
		header[72] = 0x69;
		header[73] = 0x57;
	}

	file = NULLPTR;
	data = (uint8_t *)memAlloc(imageSize);

	if (data != NULLPTR)
	{
		file = fileFunctions.create(fileName);

		if (file != NULLPTR)
		{
			if (bpp == 24)
			{
				for (row = 0; row < height; row++)
				{
					ptr = &data[rowWidth * (height - 1 - row)];

					for (x = 0; x < width; x++)
					{
						*ptr++ = image[row * width + x] >> 0;
						*ptr++ = image[row * width + x] >> 8;
						*ptr++ = image[row * width + x] >> 16;
					}
				}
			}
			else if (bpp == 32)
			{
				for (row = 0; row < height; row++)
				{
					ptr = &data[rowWidth * (height - 1 - row)];

					for (x = 0; x < width; x++)
					{
						*((uint32_t *)ptr) = image[row * width + x];
						ptr += 4;
					}
				}
			}

			fileFunctions.write(file, &header[0], bfOffBits);
			fileFunctions.write(file, data, imageSize);
			fileFunctions.close(file);
		}
		memFree(data);
	}

	return file != NULLPTR;
}

Iss360 *Iss360App::get_iss360()
{
	return m_iss360;
}
//--------------------------------------------------------------------------------------------------