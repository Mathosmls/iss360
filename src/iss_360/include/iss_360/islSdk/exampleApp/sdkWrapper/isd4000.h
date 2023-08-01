#ifndef ISD4000_H_
#define ISD4000_H_

//------------------------------------------ Includes ----------------------------------------------

#include "device.h"
#include "ahrs.h"
#include "islSdkIsd4000.h"
#include <string>

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Isd4000 : public Device
	{
	private:

		isd4000Settings_t m_settings;
		isd4000Settings_t m_pendingSettings;
		isd4000PressureCal_t m_pCal;
		isd4000TemperatureCal_t m_tCal;
		isd4000PressureSenorInfo_t m_pInfo;
		DataState m_pCalState;
		DataState m_tCalState;
		bool_t m_connectionDataSynced;
		isd4000SensorRates_t m_requestedRates;
		std::vector<std::string> m_hardCodedDepthOutputStrings;
		std::vector<std::string> m_hardCodedAhrsOutputStrings;
		std::string saveConfigPath;
		struct Isd4000Scripts
		{
			DataState varsState;
			std::vector<ScriptVar> vars;
			DeviceScript onDepth;
			DeviceScript onAhrs;
		} m_script;

		void signalSubscribersChanged(uint32_t subscriberCount);
		void connectionEvent(bool_t isConnected);
		void connectedAndReady();

	public:
		const isd4000Settings_t& settings = m_settings;
		const isd4000SensorRates_t& sensorsRates = m_requestedRates;
		const isd4000PressureCal_t& pressureCal = m_pCal;
		const isd4000TemperatureCal_t& temperatureCal = m_tCal;
		const std::vector<std::string>& hardCodedDepthOutputStrings = m_hardCodedDepthOutputStrings;
		const std::vector<std::string>& hardCodedAhrsOutputStrings = m_hardCodedAhrsOutputStrings;
		const Isd4000Scripts& scripts = m_script;

		Ahrs ahrs{ Device::id };
		GyroSensor gyro{ Device::id, 0 };
		AccelSensor accel{ Device::id, 0 };
		MagSensor mag{ Device::id, 0 };

		Signal<Isd4000&, uint64_t, real_t, real_t, real_t> onPressure{ this, &Isd4000::signalSubscribersChanged };
		Signal<Isd4000&, real_t, real_t> onTemperature{ this, &Isd4000::signalSubscribersChanged };
		Signal<Isd4000&> onScriptDataReceived;
		Signal<Isd4000&, bool_t> onSettingsUpdated;
		Signal<Isd4000&, const isd4000PressureCal_t&> onPressureCalCert;
		Signal<Isd4000&, const isd4000TemperatureCal_t&> onTemperatureCalCert;

		Isd4000(uint32_t id, const deviceInfo_t& info);
		~Isd4000();
		void handleEvent(uint64_t timeMs, uint32_t eventId, const void* data);
		void setSensorRates(const isd4000SensorRates_t& sensors);
		void setSettings(const isd4000Settings_t& settings, bool_t save);
		void measureNow();
		void setDepthScript(const uint8_t* name, const uint8_t* code);
		void setAhrsScript(const uint8_t* name, const uint8_t* code);
		void setPressureCalCert(const isd4000CalCert_t& cert);
		void setTemperatureCalCert(const isd4000CalCert_t& cert);
		bool_t getScripts();
		bool_t getCal(bool_t pressure = false, bool_t temperature = false);
		bool_t saveConfig(const str_t* fileName);
		static bool_t loadConfig(const str_t* fileName, deviceInfo_t* info, isd4000Settings_t* settings, DeviceScript* script0, DeviceScript* script1, isd4000AhrsCal_t* cal, isd4000PressureCal_t* pCal, isd4000TemperatureCal_t* tCal);
		static void getDefaultSettings(isd4000Settings_t& settings);
		bool_t hasAhrs() { return (info.config & 0x01) != 0; }
		bool_t scriptOnPingValid() { return m_script.onDepth.state == DataState::Valid; }
		bool_t scriptOnAhrsValid() { return m_script.onAhrs.state == DataState::Valid; }
		bool_t scriptVarsValid() { return m_script.varsState == DataState::Valid; }
		bool_t pressureCalValid() { return m_pCalState == DataState::Valid; }
		bool_t temperatureCalValid() { return m_tCalState == DataState::Valid; }
		real_t maxPressureRatingBar() { return m_pInfo.maxPressure; }
	};
}

//--------------------------------------------------------------------------------------------------
#endif
