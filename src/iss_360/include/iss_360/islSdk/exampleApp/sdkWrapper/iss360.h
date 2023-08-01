#ifndef ISS360_H_
#define ISS360_H_

//------------------------------------------ Includes ----------------------------------------------

#include "device.h"
#include "ahrs.h"
#include "sonarImage.h"
#include "islSdkIss360.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Iss360 : public Device
	{
	private:
		iss360Settings_t m_settings;
		iss360Settings_t m_pendingSettings;
		bool_t m_connectionDataSynced;
		iss360SensorRates_t m_requestedRates;

		void signalSubscribersChanged(uint32_t subscriberCount);
		void adcDataSignalSubscribersChanged(uint32_t subscriberCount);
		void connectionEvent(bool_t isConnected);
		void connectedAndReady();

	public:
		const iss360Settings_t &settings = m_settings;
		const iss360SensorRates_t &sensorsRates = m_requestedRates;

		iss360Ping_t last_ping;
		bool_t new_setting = true;
		SonarImage image;
		Ahrs ahrs{Device::id};
		GyroSensor gyro{Device::id, 0};
		AccelSensor accel{Device::id, 0};
		MagSensor mag{Device::id, 0};

		Signal<Iss360 &, bool_t, uint32_t> onSettingsUpdated;
		Signal<Iss360 &, bool_t, const iss360HeadHome_t &> onHeadHomed;
		Signal<Iss360 &, const iss360Ping_t &> onPingData;
		Signal<Iss360 &, const iss360AdcPingData_t &> onAdcData{this, &Iss360::adcDataSignalSubscribersChanged};
		Signal<Iss360 &, const iss360bufData_t &> onBufData;
		Signal<Iss360 &, const iss360loopBackData_t &> onLoopBackData;
		Signal<Iss360 &, const cpuPwrTemp_t &> onPwrAndTemp{this, &Iss360::signalSubscribersChanged};
		Signal<Iss360 &, const iss360EthernetTdrTest_t &> onTdrTest;

		Iss360(uint32_t id, const deviceInfo_t &info);
		~Iss360();
		void handleEvent(uint64_t timeMs, uint32_t eventId, const void *data);
		void setSensorRates(const iss360SensorRates_t &sensors);
		void setSystemSettings(const iss360SystemSettings_t &settings, bool_t save);
		void setAcousticSettings(const iss360AcousticSettings_t &settings, bool_t save);
		void setSetupSettings(const iss360SetupSettings_t &settings, bool_t save);
		void startScanning();
		void stopScanning();
		bool_t saveConfig(const str_t *fileName);
		static bool_t loadConfig(const str_t *fileName, deviceInfo_t *info, iss360Settings_t *settings, iss360AhrsCal_t *cal);
		static void getDefaultSettings(iss360Settings_t &settings);
		void startStopLogging(bool_t start);
		bool_t hasAhrs() { return (info.config & 0x01) != 0; }
		bool_t isHd() { return (info.config & 0x02) != 0; }
	};
}

//--------------------------------------------------------------------------------------------------
#endif
