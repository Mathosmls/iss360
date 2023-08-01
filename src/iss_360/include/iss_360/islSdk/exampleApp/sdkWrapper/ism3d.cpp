//------------------------------------------ Includes ----------------------------------------------

#include "ism3d.h"
#include "islSdkAhrs.h"
#include "helpers/logHelper.h"
#include "platform/maths.h"
#include "platform/mem.h"
#include "platform/debug.h"
#include "islSdkFirmware.h"

using namespace IslSdk;

//--------------------------------------------------------------------------------------------------
Ism3d::Ism3d(uint32_t id, const deviceInfo_t& info) : Device(id, info)
{
	memSet(&m_settings, 0, sizeof(ism3dSettings_t));
	memSet(&m_pendingSettings, 0, sizeof(ism3dSettings_t));
	m_connectionDataSynced = false;
	m_requestedRates.orientation = 100;
	m_requestedRates.gyro = 100;
	m_requestedRates.accel = 100;
	m_requestedRates.mag = 100;
	m_script.varsState = DataState::Invalid;

	ahrs.onData.setSubscribersChangedCallback(this, &Ism3d::signalSubscribersChanged);
	gyro.onData.setSubscribersChangedCallback(this, &Ism3d::signalSubscribersChanged);
	gyroBk.onData.setSubscribersChangedCallback(this, &Ism3d::signalSubscribersChanged);
	accel.onData.setSubscribersChangedCallback(this, &Ism3d::signalSubscribersChanged);
	accelBk.onData.setSubscribersChangedCallback(this, &Ism3d::signalSubscribersChanged);
	mag.onData.setSubscribersChangedCallback(this, &Ism3d::signalSubscribersChanged);
}
//--------------------------------------------------------------------------------------------------
Ism3d::~Ism3d()
{
}
//--------------------------------------------------------------------------------------------------
void Ism3d::connectionEvent(bool_t isConnected)
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
				islSdkIsm3dGetStringNames(id, 0);
				islSdkAhrsGetCal(id);
				islSdkIsm3dGetSettings(id);
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
void Ism3d::connectedAndReady()
{
	m_connectionDataSynced = true;
	setSensorRates(m_requestedRates);
	Device::connectionEvent(true);
}
//--------------------------------------------------------------------------------------------------
void Ism3d::signalSubscribersChanged(uint32_t subscriberCount)
{
	if (subscriberCount <= 1)
	{
		setSensorRates(m_requestedRates);
	}
}
//--------------------------------------------------------------------------------------------------
void Ism3d::handleEvent(uint64_t timeMs, uint32_t eventId, const void* data)
{
	deviceScript_t* script;
	ism3dOrientation_t* ori;
	vector3_t* vector;
	ism3dAhrsCal_t* ahrsCal;
	deviceScriptVars_t* vars;
	deviceOutputStringNames_t* strName;
	ism3dSerialTest_t* serialTest;

	switch (eventId)
	{
	case EVENT_ISM3D_ERROR:
		onError(*this, ((sdkError_t*)data)->errorCode, ((sdkError_t*)data)->msg);
		break;

	case EVENT_ISM3D_ORIENTATION_DATA:
		ori = (ism3dOrientation_t*)data;
		ahrs.onData(ahrs, ori->timeUs, ori->q, ori->magHeading, ori->turnsCount);
		break;

	case EVENT_ISM3D_GYRO_DATA:
		vector = (vector3_t*)data;
		gyro.onData(gyro, vector[0]);
		gyroBk.onData(gyro, vector[1]);
		break;

	case EVENT_ISM3D_ACCEL_DATA:
		vector = (vector3_t*)data;
		accel.onData(accel, vector[0]);
		accelBk.onData(accel, vector[1]);
		break;

	case EVENT_ISM3D_MAG_DATA:
		vector = (vector3_t*)data;
		mag.onData(mag, vector[0]);
		break;

	case EVENT_ISM3D_SETTINGS:
		m_settings = *((ism3dSettings_t*)data);

		if (!m_connectionDataSynced)
		{
			connectedAndReady();
		}
		break;

	case EVENT_ISM3D_SETTINGS_UPDATED:
		if (*((bool_t*)data))
		{
			m_settings = m_pendingSettings;
		}
		onSettingsUpdated(*this, *((bool_t*)data));
		break;

	case EVENT_ISM3D_CAL:
		ahrsCal = (ism3dAhrsCal_t*)data;
		gyro.updateCalValues(ahrsCal->gyroBias);
		gyroBk.updateCalValues(ahrsCal->gyroBiasSec);
		accel.updateCalValues(ahrsCal->accelBias, ahrsCal->accelTransform);
		accelBk.updateCalValues(ahrsCal->accelBiasSec, ahrsCal->accelTransformSec);
		mag.updateCalValues(ahrsCal->magBias, ahrsCal->magTransform);
		break;

	case EVENT_ISM3D_STRING_NAMES:
		strName = (deviceOutputStringNames_t*)data;
		m_hardCodedAhrsOutputStrings.clear();
		for (uint32_t i = 0; i < strName->count; i++)
		{
			m_hardCodedAhrsOutputStrings.emplace_back((const char*)strName->names[i]);
		}
		break;

	case EVENT_ISM3D_SCRIPT_VARS:
		vars = (deviceScriptVars_t*)data;
		m_script.vars.clear();
		for (uint32_t i = 0; i < vars->count; i++)
		{
			m_script.vars.emplace_back(&vars->var[i]);
		}
		m_script.varsState = DataState::Valid;
		if (m_script.onAhrs.state == DataState::Valid)
		{
			onScriptDataReceived(*this);
		}
		break;

	case EVENT_ISM3D_SCRIPT:
		script = (deviceScript_t*)data;
		if (script->number == 0)
		{
			m_script.onAhrs.name.assign((const char*)script->name);
			m_script.onAhrs.code.assign((const char*)script->code);
			m_script.onAhrs.state = DataState::Valid;
		}
		
		if (m_script.varsState == DataState::Valid && m_script.onAhrs.state == DataState::Valid)
		{
			if (saveConfigPath.size())
			{
				saveConfig(saveConfigPath.c_str());
				saveConfigPath.clear();
			}
			onScriptDataReceived(*this);
		}
		break;

	case EVENT_ISM3D_SERIAL_TEST:
		serialTest = (ism3dSerialTest_t*)data;
		//(serialTest->passed, serialTest->data, serialTest->size);
		break;

	default:
		break;
	}
}
//--------------------------------------------------------------------------------------------------
void Ism3d::setSensorRates(const ism3dSensorRates_t& rates)
{
	ism3dSensorRates_t toSend;

	m_requestedRates = rates;
	toSend = rates;

	if (!ahrs.onData.hasSubscribers()) toSend.orientation = 0;
	if (!gyro.onData.hasSubscribers() && !gyroBk.onData.hasSubscribers()) toSend.gyro = 0;
	if (!accel.onData.hasSubscribers() && !accelBk.onData.hasSubscribers()) toSend.accel = 0;
	if (!mag.onData.hasSubscribers()) toSend.mag = 0;

	islSdkIsm3dSetDataIntervalsMs(id, &toSend);
}
//--------------------------------------------------------------------------------------------------
void Ism3d::setSettings(const ism3dSettings_t& settings, bool_t save)
{
	m_pendingSettings = settings;
	islSdkIsm3dSetSettings(id, &settings, save);
}
//--------------------------------------------------------------------------------------------------
void Ism3d::setScript(const uint8_t* name, const uint8_t* code)
{
	islSdkIsm3dSetScript(id, 0, name, code);

	m_script.onAhrs.name.assign((const char*)name);
	m_script.onAhrs.code.assign((const char*)code);
	m_script.onAhrs.state = DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
bool_t Ism3d::getScripts()
{
	if (m_script.varsState == DataState::Invalid)
	{
		m_script.varsState = DataState::Pending;
		islSdkIsm3dGetScriptVars(id);
	}

	if (m_script.onAhrs.state == DataState::Invalid)
	{
		m_script.onAhrs.state = DataState::Pending;
		islSdkIsm3dGetScript(id, 1);
	}

	return m_script.varsState == DataState::Valid && m_script.onAhrs.state == DataState::Valid;
}
//--------------------------------------------------------------------------------------------------
bool_t Ism3d::saveConfig(const str_t* fileName)
{
	deviceScript_t script0;
	ism3dAhrsCal_t ahrsCal;

	getScripts();

	if (fileName != nullptr)
	{
		if (m_script.onAhrs.state == DataState::Valid)
		{
			script0.number = 0;
			script0.name = (const uint8_t*)m_script.onAhrs.name.c_str();
			script0.nameSize = m_script.onAhrs.name.size();
			script0.code = (const uint8_t*)m_script.onAhrs.code.c_str();
			script0.codeSize = m_script.onAhrs.code.size();

			ahrsCal.gyroBias = gyro.bias;
			ahrsCal.accelBias = accel.bias;
			ahrsCal.accelTransform = accel.transform;
			ahrsCal.magBias = mag.bias;
			ahrsCal.magTransform = mag.transform;

			return islSdkIsm3dSaveSettingsToFile(&info, &settings, &script0, &ahrsCal, fileName);
		}
		else
		{
			saveConfigPath.assign(fileName);
		}
	}
	return false;
}
//--------------------------------------------------------------------------------------------------
bool_t Ism3d::loadConfig(const str_t* fileName, deviceInfo_t* info, ism3dSettings_t* settings, DeviceScript* script0, ism3dAhrsCal_t* ahrsCal)
{
	deviceScript_t s0;
	deviceScript_t* s0Ptr = nullptr;
	bool_t ok;
	uint8_t bufName0[256];
	uint8_t bufCode0[1024];


	if (script0 != nullptr)
	{
		s0.name = &bufName0[0];
		s0.nameSize = sizeof(bufName0);
		s0.code = &bufCode0[0];
		s0.codeSize = sizeof(bufCode0);
		s0Ptr = &s0;
	}

	ok = islSdkIsm3dLoadSettingsFromFile(info, settings, s0Ptr, ahrsCal, fileName);

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

	return ok;
}
//--------------------------------------------------------------------------------------------------
void Ism3d::getDefaultSettings(ism3dSettings_t& settings)
{
	islSdkIsm3dGetDefaultSettings(&settings);
}
//--------------------------------------------------------------------------------------------------
