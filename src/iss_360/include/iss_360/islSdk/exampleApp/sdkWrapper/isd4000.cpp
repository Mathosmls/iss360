//------------------------------------------ Includes ----------------------------------------------

#include "isd4000.h"
#include "islSdkAhrs.h"
#include "helpers/logHelper.h"
#include "platform/maths.h"
#include "platform/mem.h"
#include "platform/debug.h"
#include "islSdkFirmware.h"

using namespace IslSdk;

//--------------------------------------------------------------------------------------------------
Isd4000::Isd4000(uint32_t id, const deviceInfo_t& info) : Device(id, info)
{
	memSet(&m_settings, 0, sizeof(isd4000Settings_t));
	memSet(&m_pendingSettings, 0, sizeof(isd4000Settings_t));
	memSet(&m_pCal, 0, sizeof(isd4000PressureCal_t));
	memSet(&m_tCal, 0, sizeof(isd4000TemperatureCal_t));
	m_pCalState = DataState::Invalid;
	m_tCalState = DataState::Invalid;
	m_connectionDataSynced = false;
	m_requestedRates.pressure = 100;
	m_requestedRates.orientation = 100;
	m_requestedRates.gyro = 100;
	m_requestedRates.accel = 100;
	m_requestedRates.mag = 100;
	m_requestedRates.temperature = 200;
	m_script.varsState = DataState::Invalid;

	ahrs.onData.setSubscribersChangedCallback(this, &Isd4000::signalSubscribersChanged);
	gyro.onData.setSubscribersChangedCallback(this, &Isd4000::signalSubscribersChanged);
	accel.onData.setSubscribersChangedCallback(this, &Isd4000::signalSubscribersChanged);
	mag.onData.setSubscribersChangedCallback(this, &Isd4000::signalSubscribersChanged);
}
//--------------------------------------------------------------------------------------------------
Isd4000::~Isd4000()
{
}
//--------------------------------------------------------------------------------------------------
void Isd4000::connectionEvent(bool_t isConnected)
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
				islSdkIsd4000GetStringNames(id, 0);
				islSdkIsd4000GetStringNames(id, 1);
				islSdkIsd4000GetPressureSensorinfo(id);
				islSdkAhrsGetCal(id);
				islSdkIsd4000GetSettings(id);
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
void Isd4000::connectedAndReady()
{
	m_connectionDataSynced = true;
	setSensorRates(m_requestedRates);
	Device::connectionEvent(true);
}
//--------------------------------------------------------------------------------------------------
void Isd4000::signalSubscribersChanged(uint32_t subscriberCount)
{
	if (subscriberCount <= 1)
	{
		setSensorRates(m_requestedRates);
	}
}
//--------------------------------------------------------------------------------------------------
void Isd4000::handleEvent(uint64_t timeMs, uint32_t eventId, const void* data)
{
	isd4000Pressure_t* pressure;
	isd4000Orientation_t* ori;
	isd4000Temperature_t* temp;
	isd4000AhrsCal_t* ahrsCal;
	deviceOutputStringNames_t* strName;
	deviceScriptVars_t* vars;
	deviceScript_t* script;

	switch (eventId)
	{
	case EVENT_ISD4000_ERROR:
		onError(*this, ((sdkError_t*)data)->errorCode, ((sdkError_t*)data)->msg);
		break;

	case EVENT_ISD4000_PRESSURE_DATA:
		pressure = (isd4000Pressure_t*)data;
		onPressure(*this, pressure->timeUs, pressure->bar, pressure->depthM, pressure->barRaw);
		break;

	case EVENT_ISD4000_ORIENTATION_DATA:
		ori = (isd4000Orientation_t*)data;
		ahrs.onData(ahrs, ori->timeUs, ori->q, ori->magHeading, ori->turnsCount);
		break;

	case EVENT_ISD4000_GYRO_DATA:
		gyro.onData(gyro, *((vector3_t*)data));
		break;

	case EVENT_ISD4000_ACCEL_DATA:
		accel.onData(accel, *((vector3_t*)data));
		break;

	case EVENT_ISD4000_MAG_DATA:
		mag.onData(mag, *((vector3_t*)data));
		break;

	case EVENT_ISD4000_TEMPERATURE_DATA:
		temp = (isd4000Temperature_t*)data;
		onTemperature(*this, temp->tempC, temp->tempRawC);
		break;

	case EVENT_ISD4000_SETTINGS:
		m_settings = *((isd4000Settings_t*)data);

		if (!m_connectionDataSynced)
		{
			connectedAndReady();
		}
		break;

	case EVENT_ISD4000_SETTINGS_UPDATED:
		if (*((bool_t*)data))
		{
			m_settings = m_pendingSettings;
		}
		onSettingsUpdated(*this, *((bool_t*)data));
		break;

	case EVENT_ISD4000_CAL:
		ahrsCal = (isd4000AhrsCal_t*)data;
		gyro.updateCalValues(ahrsCal->gyroBias);
		accel.updateCalValues(ahrsCal->accelBias, ahrsCal->accelTransform);
		mag.updateCalValues(ahrsCal->magBias, ahrsCal->magTransform);
		break;

	case EVENT_ISD4000_PRESSURE_CAL:
		m_pCal = *((isd4000PressureCal_t*)data);
		m_pCalState = DataState::Valid;
		if (saveConfigPath.size() && m_script.varsState == DataState::Valid && m_script.onDepth.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid && m_tCalState == DataState::Valid)
		{
			saveConfig(saveConfigPath.c_str());
			saveConfigPath.clear();
		}
		onPressureCalCert(*this, m_pCal);
		break;

	case EVENT_ISD4000_TEMPERATURE_CAL:
		m_tCal = *((isd4000TemperatureCal_t*)data);
		m_tCalState = DataState::Valid;

		if (saveConfigPath.size() && m_script.varsState == DataState::Valid && m_script.onDepth.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid && m_pCalState == DataState::Valid)
		{
			saveConfig(saveConfigPath.c_str());
			saveConfigPath.clear();
		}
		onTemperatureCalCert(*this, m_tCal);
		break;

	case EVENT_ISD4000_STRING_NAMES:
		strName = (deviceOutputStringNames_t*)data;
		if (strName->listId == 0)
		{
			m_hardCodedDepthOutputStrings.clear();
			for (uint32_t i = 0; i < strName->count; i++)
			{
				m_hardCodedDepthOutputStrings.emplace_back((const char*)strName->names[i]);
			}
		}
		else
		{
			m_hardCodedAhrsOutputStrings.clear();
			for (uint32_t i = 0; i < strName->count; i++)
			{
				m_hardCodedAhrsOutputStrings.emplace_back((const char*)strName->names[i]);
			}
		}
		break;

	case EVENT_ISD4000_SCRIPT_VARS:
		vars = (deviceScriptVars_t*)data;
		m_script.vars.clear();
		for (uint32_t i = 0; i < vars->count; i++)
		{
			m_script.vars.emplace_back(&vars->var[i]);
		}
		m_script.varsState = DataState::Valid;
		if (m_script.onDepth.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid)
		{
			onScriptDataReceived(*this);
		}
		break;

	case EVENT_ISD4000_SCRIPT:
		script = (deviceScript_t*)data;
		if (script->number == 0)
		{
			m_script.onDepth.name.assign((const char*)script->name);
			m_script.onDepth.code.assign((const char*)script->code);
			m_script.onDepth.state = DataState::Valid;
		}
		else
		{
			m_script.onAhrs.name.assign((const char*)script->name);
			m_script.onAhrs.code.assign((const char*)script->code);
			m_script.onAhrs.state = DataState::Valid;
		}
		if (m_script.varsState == DataState::Valid && m_script.onDepth.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid)
		{
			if (saveConfigPath.size() && m_pCalState == DataState::Valid && m_tCalState == DataState::Valid)
			{
				saveConfig(saveConfigPath.c_str());
				saveConfigPath.clear();
			}
			onScriptDataReceived(*this);
		}
		break;

	case EVENT_ISD4000_SENSOR_INFO:
		m_pInfo = *((isd4000PressureSenorInfo_t*)data);
		break;

	default:
		break;
	}
}
//--------------------------------------------------------------------------------------------------
void Isd4000::setSensorRates(const isd4000SensorRates_t& rates)
{
	isd4000SensorRates_t toSend;

	m_requestedRates = rates;
	toSend = rates;

	if (!onPressure.hasSubscribers()) toSend.pressure = 0;
	if (!ahrs.onData.hasSubscribers()) toSend.orientation = 0;
	if (!gyro.onData.hasSubscribers()) toSend.gyro = 0;
	if (!accel.onData.hasSubscribers()) toSend.accel = 0;
	if (!mag.onData.hasSubscribers()) toSend.mag = 0;
	if (!onTemperature.hasSubscribers()) toSend.temperature = 0;


	islSdkIsd4000SetDataIntervalsMs(id, &toSend);
}
//--------------------------------------------------------------------------------------------------
void Isd4000::setSettings(const isd4000Settings_t& settings, bool_t save)
{
	m_pendingSettings = settings;
	islSdkIsd4000SetSettings(id, &settings, save);
}
//--------------------------------------------------------------------------------------------------
void Isd4000::measureNow()
{
	isd4000SensorRates_t sensors;
	memset(&sensors, 0, sizeof(isd4000SensorRates_t));

	sensors.pressure = 1;
	sensors.temperature = 1;
	islSdkIsd4000GetData(id, &sensors);
}
//--------------------------------------------------------------------------------------------------
void Isd4000::setDepthScript(const uint8_t* name, const uint8_t* code)
{
	islSdkIsd4000SetScript(id, 0, name, code);

	m_script.onDepth.name.assign((const char*)name);
	m_script.onDepth.code.assign((const char*)code);
	m_script.onDepth.state = DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
void Isd4000::setAhrsScript(const uint8_t* name, const uint8_t* code)
{
	islSdkIsd4000SetScript(id, 1, name, code);

	m_script.onAhrs.name.assign((const char*)name);
	m_script.onAhrs.code.assign((const char*)code);
	m_script.onAhrs.state = DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
void Isd4000::setPressureCalCert(const isd4000CalCert_t& cert)
{
	islSdkIsd4000SetPressureCal(id, &cert);
	m_pCal.cal = cert;
	m_pCalState = DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
void Isd4000::setTemperatureCalCert(const isd4000CalCert_t& cert)
{
	islSdkIsd4000SetTemperatureCal(id, &cert);
	m_tCal.cal = cert;
	m_tCalState = DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
bool_t Isd4000::getScripts()
{
	if (m_script.varsState == DataState::Invalid)
	{
		m_script.varsState = DataState::Pending;
		islSdkIsd4000GetScriptVars(id);
	}

	if (m_script.onDepth.state == DataState::Invalid)
	{
		m_script.onDepth.state = DataState::Pending;
		islSdkIsd4000GetScript(id, 0);
	}

	if (m_script.onAhrs.state == DataState::Invalid)
	{
		m_script.onAhrs.state = DataState::Pending;
		islSdkIsd4000GetScript(id, 1);
	}

	return m_script.varsState == DataState::Valid && m_script.onDepth.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid;
}

//--------------------------------------------------------------------------------------------------
bool_t Isd4000::getCal(bool_t pressure, bool_t temperature)
{
	if (m_pCalState == DataState::Invalid || pressure)
	{
		m_pCalState = DataState::Pending;
		islSdkIsd4000GetPressureCal(id);
	}

	if (m_tCalState == DataState::Invalid || temperature)
	{
		m_tCalState = DataState::Pending;
		islSdkIsd4000GetTemperatureCal(id);
	}

	return m_pCalState == DataState::Valid && m_tCalState == DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
bool_t Isd4000::saveConfig(const str_t* fileName)
{
	deviceScript_t script0, script1;
	isd4000AhrsCal_t ahrsCal;

	getCal();
	getScripts();

	if (fileName != nullptr)
	{
		if (m_script.onDepth.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid && m_pCalState == DataState::Valid && m_tCalState == DataState::Valid)
		{
			script0.number = 0;
			script0.name = (const uint8_t*)m_script.onDepth.name.c_str();
			script0.nameSize = m_script.onDepth.name.size();
			script0.code = (const uint8_t*)m_script.onDepth.code.c_str();
			script0.codeSize = m_script.onDepth.code.size();

			script1.number = 1;
			script1.name = (const uint8_t*)m_script.onAhrs.name.c_str();
			script1.nameSize = m_script.onAhrs.name.size();
			script1.code = (const uint8_t*)m_script.onAhrs.code.c_str();
			script1.codeSize = m_script.onAhrs.code.size();

			ahrsCal.gyroBias = gyro.bias;
			ahrsCal.accelBias = accel.bias;
			ahrsCal.accelTransform = accel.transform;
			ahrsCal.magBias = mag.bias;
			ahrsCal.magTransform = mag.transform;

			return islSdkIsd4000SaveSettingsToFile(&info, &settings, &script0, &script1, &ahrsCal, &m_pCal, &m_tCal, fileName);
		}
		else
		{
			saveConfigPath.assign(fileName);
		}
	}
	return false;
}
//--------------------------------------------------------------------------------------------------
bool_t Isd4000::loadConfig(const str_t* fileName, deviceInfo_t* info, isd4000Settings_t* settings, DeviceScript* script0, DeviceScript* script1, isd4000AhrsCal_t* ahrsCal, isd4000PressureCal_t* pCal, isd4000TemperatureCal_t* tCal)
{
	deviceScript_t s0, s1;
	deviceScript_t* s0Ptr = nullptr;
	deviceScript_t* s1Ptr = nullptr;
	bool_t ok;
	uint8_t bufName0[256];
	uint8_t bufCode0[1024];
	uint8_t bufName1[256];
	uint8_t bufCode1[1024];

	if (script0 != nullptr)
	{
		s0.name = &bufName0[0];
		s0.nameSize = sizeof(bufName0);
		s0.code = &bufCode0[0];
		s0.codeSize = sizeof(bufCode0);
		s0Ptr = &s0;
	}

	if (script1 != nullptr)
	{
		s1.name = &bufName1[0];
		s1.nameSize = sizeof(bufName1);
		s1.code = &bufCode1[0];
		s1.codeSize = sizeof(bufCode1);
		s1Ptr = &s1;
	}

	ok = islSdkIsd4000LoadSettingsFromFile(info, settings, s0Ptr, s1Ptr, ahrsCal, pCal, tCal, fileName);

	if (script0 != nullptr)
	{
		if (ok)
		{
			script0->state = DataState::Valid;
			script0->name.assign((const char*)s0.name);
			script0->code.assign((const char*)s0.code);
		}
		else
		{
			script0->state = DataState::Invalid;
		}
	}

	if (script1 != nullptr)
	{
		if (ok)
		{
			script1->state = DataState::Valid;
			script1->name.assign((const char*)s1.name);
			script1->code.assign((const char*)s1.code);
		}
		else
		{
			script1->state = DataState::Invalid;
		}
	}

	return ok;
}
//--------------------------------------------------------------------------------------------------
void Isd4000::getDefaultSettings(isd4000Settings_t& settings)
{
	islSdkIsd4000GetDefaultSettings(&settings);
}
//--------------------------------------------------------------------------------------------------
