//------------------------------------------ Includes ----------------------------------------------

#include "iss360.h"
#include "islSdkAhrs.h"
#include "helpers/logHelper.h"
#include "platform/maths.h"
#include "platform/mem.h"
#include "platform/debug.h"
#include "islSdkFirmware.h"

using namespace IslSdk;

//--------------------------------------------------------------------------------------------------
Iss360::Iss360(uint32_t id, const deviceInfo_t& info) : Device(id, info)
{
	memSet(&m_settings, 0, sizeof(iss360Settings_t));
	memSet(&m_pendingSettings, 0, sizeof(iss360Settings_t));
	m_connectionDataSynced = false;
	
	m_requestedRates.orientation = 100;
	m_requestedRates.gyro = 100;
	m_requestedRates.accel = 100;
	m_requestedRates.mag = 100;
	m_requestedRates.voltageAndTemp = 1000;

	ahrs.onData.setSubscribersChangedCallback(this, &Iss360::signalSubscribersChanged);
	gyro.onData.setSubscribersChangedCallback(this, &Iss360::signalSubscribersChanged);
	accel.onData.setSubscribersChangedCallback(this, &Iss360::signalSubscribersChanged);
	mag.onData.setSubscribersChangedCallback(this, &Iss360::signalSubscribersChanged);
}
//--------------------------------------------------------------------------------------------------
Iss360::~Iss360()
{
}
//--------------------------------------------------------------------------------------------------
void Iss360::connectionEvent(bool_t isConnected)
{
	if (isConnected)
	{
		if (info.bootloaderMode)
		{
			Device::connectionEvent(true);
		}
		else
		{
			if (!m_connectionDataSynced)
			{
				islSdkAhrsGetCal(id);
				islSdkIss360GetSettings(id);
			}
			else
			{
				connectedAndReady();
			}
		}
	}
	else
	{
		Device::connectionEvent(false);
	}
}
//--------------------------------------------------------------------------------------------------
void Iss360::connectedAndReady()
{
	m_connectionDataSynced = true;
	setSensorRates(m_requestedRates);
	islSdkIss360SetOptions(id, onAdcData.hasSubscribers());
	Device::connectionEvent(true);
}
//--------------------------------------------------------------------------------------------------
void Iss360::signalSubscribersChanged(uint32_t subscriberCount)
{
	if (subscriberCount <= 1)
	{
		setSensorRates(m_requestedRates);
	}
}
//--------------------------------------------------------------------------------------------------
void Iss360::adcDataSignalSubscribersChanged(uint32_t subscriberCount)
{
	if (subscriberCount <= 1)
	{
		islSdkIss360SetOptions(id, subscriberCount == 1);
	}
}
//--------------------------------------------------------------------------------------------------
void Iss360::handleEvent(uint64_t timeMs, uint32_t eventId, const void* data)
{
	iss360Orientation_t* ori;
	iss360AhrsCal_t* ahrsCal;
	uint32_t txPulseLengthMm;

	switch (eventId)
	{
	case EVENT_ISS360_ERROR:
		onError(*this, ((sdkError_t*)data)->errorCode, ((sdkError_t*)data)->msg);
		break;

	case EVENT_ISS360_ORIENTATION_DATA:
		ori = (iss360Orientation_t*)data;
		ahrs.onData(ahrs, 0, ori->q, ori->magHeading, ori->turnsCount);
		if (m_log.active)
		{
			logHelperLogData(m_log.logger->id, m_log.trackId, info.pid, EVENT_ISS360_ORIENTATION_DATA, data);
		}
		break;

	case EVENT_ISS360_GYRO_DATA:
		gyro.onData(gyro, *((vector3_t*)data));
		break;

	case EVENT_ISS360_ACCEL_DATA:
		accel.onData(accel, *((vector3_t*)data));
		break;

	case EVENT_ISS360_MAG_DATA:
		mag.onData(mag, *((vector3_t*)data));
		break;

	case EVENT_ISS360_SETTINGS:
		m_settings = *((iss360Settings_t*)data);

		if (!m_connectionDataSynced)
		{
			if (m_log.logger != nullptr && m_log.active)
			{
				logHelperLogData(m_log.logger->id, m_log.trackId, info.pid, EVENT_ISS360_SETTINGS, &m_settings);
			}
			connectedAndReady();
		}
		break;

	case EVENT_ISS360_SYSTEM_SETTINGS_UPDATED:
		if (*((bool_t*)data))
		{
			m_settings.system = m_pendingSettings.system;

			if (m_log.active)
			{
				logHelperLogData(m_log.logger->id, m_log.trackId, info.pid, EVENT_ISS360_SETTINGS, &m_settings);
			}
		}
		onSettingsUpdated(*this, *((bool_t*)data), 0);
		break;

	case EVENT_ISS360_ACOUSTIC_SETTINGS_UPDATED:	
		if (*((bool_t*)data))
		{
			m_settings.acoustic = m_pendingSettings.acoustic;

			if (m_log.active)
			{
				logHelperLogData(m_log.logger->id, m_log.trackId, info.pid, EVENT_ISS360_SETTINGS, &m_settings);
			}
		}
		onSettingsUpdated(*this, *((bool_t*)data), 1);
		break;

	case EVENT_ISS360_SETUP_SETTINGS_UPDATED:
		if (*((bool_t*)data))
		{
			m_settings.setup = m_pendingSettings.setup;

			if (m_log.active)
			{
				logHelperLogData(m_log.logger->id, m_log.trackId, info.pid, EVENT_ISS360_SETTINGS, &m_settings);
			}
		}
		onSettingsUpdated(*this, *((bool_t*)data), 2);
		break;

	case EVENT_ISS360_CAL:
		ahrsCal = (iss360AhrsCal_t*)data;
		gyro.updateCalValues(ahrsCal->gyroBias);
		accel.updateCalValues(ahrsCal->accelBias, ahrsCal->accelTransform);
		mag.updateCalValues(ahrsCal->magBias, ahrsCal->magTransform);
		break;

	case EVENT_ISS360_HOMED:
		onHeadHomed(*this, ((iss360HeadHome_t*)data)->state == ISS360_HOMED_OK, *((iss360HeadHome_t*)data));
		break;

	case EVENT_ISS360_PING_DATA:
		if (m_log.active)
		{
			logHelperLogData(m_log.logger->id, m_log.trackId, info.pid, EVENT_ISS360_PING_DATA, data);
		}
		
		txPulseLengthMm = (uint32_t)(m_settings.setup.speedOfSound * m_settings.acoustic.txPulseWidthUs * 0.001 * 0.5);
		if (txPulseLengthMm < 150)
		{
			txPulseLengthMm = 150;
		}
		image.addData(*((iss360Ping_t*)data), txPulseLengthMm);

		onPingData(*this, *((iss360Ping_t*)data));
		break;

	case EVENT_ISS360_ADC_DATA:
		onAdcData(*this, *((iss360AdcPingData_t*)data));
		break;

	case EVENT_ISS360_BUF_DATA:
		onBufData(*this, *((iss360bufData_t*)data));
		break;

	case EVENT_ISS360_LOOPBACK_DATA:
		onLoopBackData(*this, *((iss360loopBackData_t*)data));
		break;

	case EVENT_ISS360_TDR_TEST:
		onTdrTest(*this, *((iss360EthernetTdrTest_t*)data));
		break;

	case EVENT_ISS360_CPU_TEMP_PWR:
		onPwrAndTemp(*this, *((cpuPwrTemp_t*)data));
		break;

	default:
		break;
	}
}
//--------------------------------------------------------------------------------------------------
void Iss360::setSensorRates(const iss360SensorRates_t& rates)
{
	iss360SensorRates_t toSend;

	m_requestedRates = rates;
	toSend = rates;

	if (!ahrs.onData.hasSubscribers()) toSend.orientation = 0;
	if (!gyro.onData.hasSubscribers()) toSend.gyro = 0;
	if (!accel.onData.hasSubscribers()) toSend.accel = 0;
	if (!mag.onData.hasSubscribers()) toSend.mag = 0;
	if (!onPwrAndTemp.hasSubscribers()) toSend.voltageAndTemp = 0;

	islSdkIss360SetDataIntervalsMs(id, &toSend);
}
//--------------------------------------------------------------------------------------------------
void Iss360::setSystemSettings(const iss360SystemSettings_t& settings, bool_t save)
{
	m_pendingSettings.system = settings;
	islSdkIss360SetSystemSettings(id, &settings, save);
}
//--------------------------------------------------------------------------------------------------
void Iss360::setAcousticSettings(const iss360AcousticSettings_t& settings, bool_t save)
{
	m_pendingSettings.acoustic = settings;
	islSdkIss360SetAcousticSettings(id, &settings, save);
}
//--------------------------------------------------------------------------------------------------
void Iss360::setSetupSettings(const iss360SetupSettings_t& settings, bool_t save)
{
	m_pendingSettings.setup = settings;
	islSdkIss360SetSetupSettings(id, &settings, save);
}
//--------------------------------------------------------------------------------------------------
void Iss360::startScanning()
{
	islSdkIss360StartStop(id, TRUE);
}
//--------------------------------------------------------------------------------------------------
void Iss360::stopScanning()
{
	islSdkIss360StartStop(id, FALSE);
}
//--------------------------------------------------------------------------------------------------
bool_t Iss360::saveConfig(const str_t* fileName)
{
	iss360AhrsCal_t ahrsCal;

	ahrsCal.gyroBias = gyro.bias;
	ahrsCal.accelBias = accel.bias;
	ahrsCal.accelTransform = accel.transform;
	ahrsCal.magBias = mag.bias;
	ahrsCal.magTransform = mag.transform;

	return islSdkIss360SaveSettingsToFile(&info, &settings, &ahrsCal, fileName);
}
//--------------------------------------------------------------------------------------------------
bool_t Iss360::loadConfig(const str_t* fileName, deviceInfo_t* info, iss360Settings_t* settings, iss360AhrsCal_t* ahrsCal)
{
	return islSdkIss360LoadSettingsFromFile(info, settings, ahrsCal, fileName);
}
//--------------------------------------------------------------------------------------------------
void Iss360::getDefaultSettings(iss360Settings_t& settings)
{
	islSdkIss360GetDefaultSettings(&settings);
}
//--------------------------------------------------------------------------------------------------
void Iss360::startStopLogging(bool_t start)
{
	if (start && m_log.logger != nullptr && isConnected)
	{
		logHelperLogData(m_log.logger->id, m_log.trackId, info.pid, EVENT_ISS360_SETTINGS, &m_settings);
	}

	Device::startStopLogging(start);
}
//--------------------------------------------------------------------------------------------------
