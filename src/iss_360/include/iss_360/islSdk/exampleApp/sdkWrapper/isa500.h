#ifndef ISA500_H_
#define ISA500_H_

//------------------------------------------ Includes ----------------------------------------------

#include "device.h"
#include "ahrs.h"
#include "islSdkIsa500.h"
#include <string>

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Isa500 : public Device
	{
	private:

		isa500Settings_t m_settings;
		isa500Settings_t m_pendingSettings;
		bool_t m_connectionDataSynced;
		isa500SensorRates_t m_requestedRates;
		std::vector<std::string> m_hardCodedPingOutputStrings;
		std::vector<std::string> m_hardCodedAhrsOutputStrings;
		std::string saveConfigPath;
		struct Isa500Scripts
		{
			DataState varsState;
			std::vector<ScriptVar> vars;
			DeviceScript onPing;
			DeviceScript onAhrs;
		} m_script;

		void signalSubscribersChanged(uint32_t subscriberCount);
		void connectionEvent(bool_t isConnected);
		void connectedAndReady();

	public:
		const isa500Settings_t& settings = m_settings;
		const isa500SensorRates_t& sensorsRates = m_requestedRates;
		const std::vector<std::string>& hardCodedPingOutputStrings = m_hardCodedPingOutputStrings;
		const std::vector<std::string>& hardCodedAhrsOutputStrings = m_hardCodedAhrsOutputStrings;
		const Isa500Scripts& scripts = m_script;

		Ahrs ahrs{ Device::id};
		GyroSensor gyro{ Device::id, 0 };
		AccelSensor accel{ Device::id, 0 };
		MagSensor mag{ Device::id, 0 };

		Signal<Isa500&, const isa500Echoes_t&> onEcho{this, &Isa500::signalSubscribersChanged};
		Signal<Isa500&, real_t> onTemperature{ this, &Isa500::signalSubscribersChanged};
		Signal<Isa500&, real_t> onVoltage{ this, &Isa500::signalSubscribersChanged};
		Signal<Isa500&, bool_t> onTrigger;
		Signal<Isa500&> onScriptDataReceived;
		Signal<Isa500&, bool_t> onSettingsUpdated;
		Signal<Isa500&, const isa500TestWaveform_t&> onPingTest;

		Isa500(uint32_t id, const deviceInfo_t& info);
		~Isa500();
		void handleEvent(uint64_t timeMs, uint32_t eventId, const void* data);
		void setSensorRates(const isa500SensorRates_t& sensors);
		void setSettings(const isa500Settings_t& settings, bool_t save);
		void pingNow();
		void setPingScript(const uint8_t* name, const uint8_t* code);
		void setAhrsScript(const uint8_t* name, const uint8_t* code);
		bool_t getScripts();
		bool_t saveConfig(const str_t* fileName);
		static bool_t loadConfig(const str_t* fileName, deviceInfo_t* info, isa500Settings_t* settings, DeviceScript* script0, DeviceScript* script1, isa500AhrsCal_t* cal);
		static void getDefaultSettings(isa500Settings_t& settings);
		bool_t hasAhrs() { return (info.config & 0x01) != 0; }
		bool_t hasRightAngleTransducer() { return (info.config & 0x02) != 0; }
		bool_t hasCurrentLoop() { return (info.config & 0x80) == 0; }
		bool_t scriptOnPingValid() { return m_script.onPing.state == DataState::Valid; }
		bool_t scriptOnAhrsValid() { return m_script.onAhrs.state == DataState::Valid; }
		bool_t scriptVarsValid() { return m_script.varsState == DataState::Valid; }
	};
}

//--------------------------------------------------------------------------------------------------
#endif
