#ifndef ISM3D_H_
#define ISM3D_H_

//------------------------------------------ Includes ----------------------------------------------

#include "device.h"
#include "ahrs.h"
#include "islSdkIsm3d.h"
#include <string>

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Ism3d : public Device
	{
	private:

		ism3dSettings_t m_settings;
		ism3dSettings_t m_pendingSettings;
		bool_t m_connectionDataSynced;
		ism3dSensorRates_t m_requestedRates;
		std::vector<std::string> m_hardCodedAhrsOutputStrings;
		std::string saveConfigPath;
		struct ism3dScripts
		{
			DataState varsState;
			std::vector<ScriptVar> vars;
			DeviceScript onAhrs;
		} m_script;

		void signalSubscribersChanged(uint32_t subscriberCount);
		void connectionEvent(bool_t isConnected);
		void connectedAndReady();

	public:
		const ism3dSettings_t& settings = m_settings;
		const ism3dSensorRates_t& sensorsRates = m_requestedRates;
		const std::vector<std::string>& hardCodedAhrsOutputStrings = m_hardCodedAhrsOutputStrings;
		const ism3dScripts& scripts = m_script;

		Ahrs ahrs{ Device::id };
		GyroSensor gyro{ Device::id, 0 };
		GyroSensor gyroBk{ Device::id, 1 };
		AccelSensor accel{ Device::id, 0 };
		AccelSensor accelBk{ Device::id, 1 };
		MagSensor mag{ Device::id, 0 };

		Signal<Ism3d&> onScriptDataReceived;
		Signal<Ism3d&, bool_t> onSettingsUpdated;


		Ism3d(uint32_t id, const deviceInfo_t& info);
		~Ism3d();
		void handleEvent(uint64_t timeMs, uint32_t eventId, const void* data);
		void setSensorRates(const ism3dSensorRates_t& sensors);
		void setSettings(const ism3dSettings_t& settings, bool_t save);
		void setScript(const uint8_t* name, const uint8_t* code);
		bool_t getScripts();
		bool_t saveConfig(const str_t* fileName);
		static bool_t loadConfig(const str_t* fileName, deviceInfo_t* info, ism3dSettings_t* settings, DeviceScript* script0, ism3dAhrsCal_t* ahrsCal);
		static void getDefaultSettings(ism3dSettings_t& settings);
		bool_t scriptOnAhrsValid() { return m_script.onAhrs.state == DataState::Valid; }
		bool_t scriptVarsValid() { return m_script.varsState == DataState::Valid; }
	};
}

//--------------------------------------------------------------------------------------------------
#endif
