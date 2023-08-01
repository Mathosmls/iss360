#ifndef ISS360APP_H_
#define ISS360APP_H_

//------------------------------------------ Includes ----------------------------------------------

#include "iss360.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Iss360App
	{
	public:
		Iss360App(void);
		~Iss360App(void);
		void disconnectAllEvents(void);
		void setDevice(Iss360* device);
		void doTask(int32_t key, const str_t* path);

		Slot<Device&, uint32_t, const utf8_t*> slotError{ this, &Iss360App::callbackError };
		Slot<Device&> slotDelete{ this, &Iss360App::callbackDeleteted };
		Slot<Device&> slotConnect{ this, &Iss360App::callbackConnect };
		Slot<Device&> slotDisconnect{ this, &Iss360App::callbackDisconnect };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortAdded{ this, &Iss360App::callbackPortAdded };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortChanged{ this, &Iss360App::callbackPortChanged };
		Slot<Device&, SysPort&> slotPortRemoved{ this, &Iss360App::callbackPortRemoved };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortSelected{ this, &Iss360App::callbackPortSelected };
		Slot<Device&, const deviceInfo_t&> slotDeviceInfo{ this, &Iss360App::callbackDeviceInfo };
		Slot<Device&, const deviceStats_t&> slotPacketStats{ this, &Iss360App::callbackPacketStats };
		Slot<Device&, const bootloaderProgress_t&> slotBootloader{ this, &Iss360App::callbackBootloader };

		Slot<Ahrs&, uint64_t, const quaternion_t&, real_t, real_t> slotAhrsData{ this, &Iss360App::callbackAhrs };
		Slot<GyroSensor&, const vector3_t&> slotGyroData{ this, &Iss360App::callbackGyroData };
		Slot<AccelSensor&, const vector3_t&> slotAccelData{ this, &Iss360App::callbackAccelData };
		Slot<MagSensor&, const vector3_t&> slotMagData{ this, &Iss360App::callbackMagData };
		Slot<AccelSensor&, uint32_t, const vector3_t&, uint8_t> slotAccelCal{ this, &Iss360App::callbackAccelCal };

		Slot<Iss360&, bool_t, uint32_t> slotSettingsUpdated{ this, &Iss360App::callbackSettingsUpdated };
		Slot<Iss360&, const cpuPwrTemp_t& > slotPwrAndTemp{ this, &Iss360App::callbackPwrAndTemp };
		Slot<Iss360&, const iss360Ping_t&> slotPingData{ this, &Iss360App::callbackPingData };
		Slot<Iss360&, const iss360AdcPingData_t&> slotAdcData{ this, &Iss360App::callbackAdcData };

		SonarImageBuf m_sonarImage;
		Iss360 *get_iss360();

	private:
		Iss360* m_iss360;
		SonarImageBuf m_sonarTexture;
		Palette m_palette;
		uint32_t m_pingCount;

		void callbackError(Device& device, uint32_t errorCode, const utf8_t* msg);
		void callbackDeleteted(Device& device);
		void callbackConnect(Device& device);
		void callbackDisconnect(Device& device);
		void callbackPortAdded(Device& device, SysPort& port, const connectionMeta_t& meta);
		void callbackPortChanged(Device& device, SysPort& port, const connectionMeta_t& meta);
		void callbackPortRemoved(Device& device, SysPort& port);
		void callbackPortSelected(Device& device, SysPort& port, const connectionMeta_t& meta);
		void callbackDeviceInfo(Device& device, const deviceInfo_t& info);
		void callbackPacketStats(Device& device, const deviceStats_t& stats);
		void callbackBootloader(Device& device, const bootloaderProgress_t& progress);

		void callbackAhrs(Ahrs& ahrs, uint64_t timeUs, const quaternion_t& q, real_t magHeading, real_t turnsCount);
		void callbackGyroData(GyroSensor& gyro, const vector3_t& v);
		void callbackAccelData(AccelSensor& accel, const vector3_t& v);
		void callbackMagData(MagSensor& mag, const vector3_t& v);
		void callbackAccelCal(AccelSensor& accel, uint32_t axis, const vector3_t& v, uint8_t progress);

		void callbackSettingsUpdated(Iss360& iss360, bool_t ok, uint32_t set);
		void callbackPwrAndTemp(Iss360& iss360, const cpuPwrTemp_t& data);
		void callbackPingData(Iss360& iss360, const iss360Ping_t& data);
		void callbackAdcData(Iss360& iss360, const iss360AdcPingData_t& data);
	};
}

//--------------------------------------------------------------------------------------------------
#endif