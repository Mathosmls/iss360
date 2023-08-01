//------------------------------------------ Includes ----------------------------------------------

#include "isa500.h"
#include "islSdkAhrs.h"
#include "helpers/logHelper.h"
#include "platform/maths.h"
#include "platform/debug.h"
#include "platform/mem.h"
#include "islSdkFirmware.h"
#include <memory>

using namespace IslSdk;

//--------------------------------------------------------------------------------------------------
Isa500::Isa500(uint32_t id, const deviceInfo_t& info) : Device(id, info)
{
	memSet(&m_settings, 0, sizeof(isa500Settings_t));
	memSet(&m_pendingSettings, 0, sizeof(isa500Settings_t));
	m_connectionDataSynced = false;
	m_requestedRates.ping = 1000;
	m_requestedRates.orientation = 100;
	m_requestedRates.gyro = 100;
	m_requestedRates.accel = 100;
	m_requestedRates.mag = 100;
	m_requestedRates.temperature = 1000;
	m_requestedRates.voltage = 1000;
	m_script.varsState = DataState::Invalid;

	ahrs.onData.setSubscribersChangedCallback(this, &Isa500::signalSubscribersChanged);
	gyro.onData.setSubscribersChangedCallback(this, &Isa500::signalSubscribersChanged);
	accel.onData.setSubscribersChangedCallback(this, &Isa500::signalSubscribersChanged);
	mag.onData.setSubscribersChangedCallback(this, &Isa500::signalSubscribersChanged);
}
//--------------------------------------------------------------------------------------------------
Isa500::~Isa500()
{
}
//--------------------------------------------------------------------------------------------------
void Isa500::connectionEvent(bool_t isConnected)
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
				islSdkIsa500GetStringNames(id, 0);
				islSdkIsa500GetStringNames(id, 1);
				islSdkAhrsGetCal(id);
				islSdkIsa500GetSettings(id);
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
void Isa500::connectedAndReady()
{
	m_connectionDataSynced = true;
	setSensorRates(m_requestedRates);
	Device::connectionEvent(true);
}
//--------------------------------------------------------------------------------------------------
void Isa500::signalSubscribersChanged(uint32_t subscriberCount)
{
	if (subscriberCount <= 1)
	{
		setSensorRates(m_requestedRates);
	}
}
//--------------------------------------------------------------------------------------------------
void Isa500::handleEvent(uint64_t timeMs, uint32_t eventId, const void* data)
{
	deviceScript_t* script;
	isa500Orientation_t* ori;
	isa500Echoes_t* echoes;
	isa500AhrsCal_t* ahrsCal;
	deviceScriptVars_t* vars;
	deviceOutputStringNames_t* strName;

	switch (eventId)
	{
	case EVENT_ISA500_ERROR:
		onError(*this, ((sdkError_t*)data)->errorCode, ((sdkError_t*)data)->msg);
		break;

	case EVENT_ISA500_ECHO_DATA:
		echoes = (isa500Echoes_t*)data;
		onEcho(*this, *echoes);
		break;

	case EVENT_ISA500_ORIENTATION_DATA:
		ori = (isa500Orientation_t*)data;
		ahrs.onData(ahrs, ori->timeUs, ori->q, ori->magHeading, ori->turnsCount);
		break;

	case EVENT_ISA500_GYRO_DATA:
		gyro.onData(gyro,*((vector3_t*)data));
		break;

	case EVENT_ISA500_ACCEL_DATA:
		accel.onData(accel, *((vector3_t*)data));
		break;

	case EVENT_ISA500_MAG_DATA:
		mag.onData(mag, *((vector3_t*)data));
		break;

	case EVENT_ISA500_TEMPERATURE_DATA:
		onTemperature(*this, *((real_t*)data));
		break;

	case EVENT_ISA500_VOLTAGE_DATA:
		onVoltage(*this, *((real_t*)data));
		break;

	case EVENT_ISA500_TRIGGER_DATA:
		onTrigger(*this, *((bool_t*)data));
		break;

	case EVENT_ISA500_SETTINGS:
		m_settings = *((isa500Settings_t*)data);

		if (!m_connectionDataSynced)
		{
			connectedAndReady();
		}
		break;

	case EVENT_ISA500_SETTINGS_UPDATED:
		if (*((bool_t*)data))
		{
			m_settings = m_pendingSettings;
		}
		onSettingsUpdated(*this, *((bool_t*)data));
		break;

	case EVENT_ISA500_CAL:
		ahrsCal = (isa500AhrsCal_t*)data;
		gyro.updateCalValues(ahrsCal->gyroBias);
		accel.updateCalValues(ahrsCal->accelBias, ahrsCal->accelTransform);
		mag.updateCalValues(ahrsCal->magBias, ahrsCal->magTransform);
		break;

	case EVENT_ISA500_STRING_NAMES:
		strName = (deviceOutputStringNames_t*)data;
		if (strName->listId == 0)
		{
			m_hardCodedPingOutputStrings.clear();
			for (uint32_t i = 0; i < strName->count; i++)
			{
				m_hardCodedPingOutputStrings.emplace_back((const char*)strName->names[i]);
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

	case EVENT_ISA500_SCRIPT_VARS:
		vars = (deviceScriptVars_t*)data;
		m_script.vars.clear();
		for (uint32_t i = 0; i < vars->count; i++)
		{
			m_script.vars.emplace_back(&vars->var[i]);
		}
		m_script.varsState = DataState::Valid;
		if (m_script.onPing.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid)
		{
			onScriptDataReceived(*this);
		}
		break;

	case EVENT_ISA500_SCRIPT:
		script = (deviceScript_t*)data;
		if (script->number == 0)
		{
			m_script.onPing.name.assign((const char*)script->name);
			m_script.onPing.code.assign((const char*)script->code);
			m_script.onPing.state = DataState::Valid;
		}
		else
		{
			m_script.onAhrs.name.assign((const char*)script->name);
			m_script.onAhrs.code.assign((const char*)script->code);
			m_script.onAhrs.state = DataState::Valid;
		}
		if (m_script.varsState == DataState::Valid && m_script.onPing.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid)
		{
			if (saveConfigPath.size())
			{
				saveConfig(saveConfigPath.c_str());
				saveConfigPath.clear();
			}
			onScriptDataReceived(*this);
		}
		break;

	case EVENT_ISA500_PING_TEST:
		onPingTest(*this, *((isa500TestWaveform_t*)data));
		break;

	default:
		break;
	}
}
//--------------------------------------------------------------------------------------------------
void Isa500::setSensorRates(const isa500SensorRates_t& rates)
{
	isa500SensorRates_t toSend;

	m_requestedRates = rates;
	toSend = rates;

	if (!onEcho.hasSubscribers()) toSend.ping = 0;
	if (!ahrs.onData.hasSubscribers()) toSend.orientation = 0;
	if (!gyro.onData.hasSubscribers()) toSend.gyro = 0;
	if (!accel.onData.hasSubscribers()) toSend.accel = 0;
	if (!mag.onData.hasSubscribers()) toSend.mag = 0;
	if (!onTemperature.hasSubscribers()) toSend.temperature = 0;
	if (!onVoltage.hasSubscribers()) toSend.voltage = 0;

	islSdkIsa500SetDataIntervalsMs(id, &toSend);
}
//--------------------------------------------------------------------------------------------------
void Isa500::setSettings(const isa500Settings_t& settings, bool_t save)
{
	m_pendingSettings = settings;
	islSdkIsa500SetSettings(id, &settings, save);
}
//--------------------------------------------------------------------------------------------------
void Isa500::pingNow()
{
	isa500SensorRates_t sensors;
	memset(&sensors, 0, sizeof(isa500SensorRates_t));

	sensors.ping = 1;
	islSdkIsa500GetData(id, &sensors);
}
//--------------------------------------------------------------------------------------------------
void Isa500::setPingScript(const uint8_t* name, const uint8_t* code)
{
	islSdkIsa500SetScript(id, 0, name, code);

	m_script.onPing.name.assign((const char*)name);
	m_script.onPing.code.assign((const char*)code);
	m_script.onPing.state = DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
void Isa500::setAhrsScript(const uint8_t* name, const uint8_t* code)
{
	islSdkIsa500SetScript(id, 1, name, code);

	m_script.onAhrs.name.assign((const char*)name);
	m_script.onAhrs.code.assign((const char*)code);
	m_script.onAhrs.state = DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
bool_t Isa500::getScripts()
{
	if (m_script.varsState == DataState::Invalid)
	{
		m_script.varsState = DataState::Pending;
		islSdkIsa500GetScriptVars(id);
	}

	if (m_script.onPing.state == DataState::Invalid)
	{
		m_script.onPing.state = DataState::Pending;
		islSdkIsa500GetScript(id, 0);
	}

	if (m_script.onAhrs.state == DataState::Invalid)
	{
		m_script.onAhrs.state = DataState::Pending;
		islSdkIsa500GetScript(id, 1);
	}

	return m_script.varsState == DataState::Valid && m_script.onPing.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
bool_t Isa500::saveConfig(const str_t* fileName)
{
	deviceScript_t script0, script1;
	isa500AhrsCal_t ahrsCal;

	getScripts();

	if (fileName != nullptr)
	{
		if (m_script.onPing.state == DataState::Valid && m_script.onAhrs.state == DataState::Valid)
		{
			script0.number = 0;
			script0.name = (const uint8_t*)m_script.onPing.name.c_str();
			script0.nameSize = m_script.onPing.name.size();
			script0.code = (const uint8_t*)m_script.onPing.code.c_str();
			script0.codeSize = m_script.onPing.code.size();

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

			return islSdkIsa500SaveSettingsToFile(&info, &settings, &script0, &script1, &ahrsCal, fileName);
		}
		else
		{
			saveConfigPath.assign(fileName);
		}
	}

	return false;
}
//--------------------------------------------------------------------------------------------------
bool_t Isa500::loadConfig(const str_t* fileName, deviceInfo_t* info, isa500Settings_t* settings, DeviceScript* script0, DeviceScript* script1, isa500AhrsCal_t* ahrsCal)
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

	ok = islSdkIsa500LoadSettingsFromFile(info, settings, s0Ptr, s1Ptr, ahrsCal, fileName);

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
void Isa500::getDefaultSettings(isa500Settings_t& settings)
{
	islSdkIsa500GetDefaultSettings(&settings);
}
//--------------------------------------------------------------------------------------------------
