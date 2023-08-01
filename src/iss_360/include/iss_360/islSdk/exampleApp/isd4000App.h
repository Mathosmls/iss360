#ifndef ISD4000APP_H_
#define ISD4000APP_H_

//------------------------------------------ Includes ----------------------------------------------

#include "isd4000.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Isd4000App
	{
	public:
		Isd4000App(void);
		~Isd4000App(void);
		void disconnectAllEvents(void);
		void setDevice(Isd4000* device);
		void doTask(int32_t key, const str_t* path);

		Slot<Device&, uint32_t, const utf8_t*> slotError{ this, &Isd4000App::callbackError };
		Slot<Device&> slotDelete{ this, &Isd4000App::callbackDeleteted };
		Slot<Device&> slotConnect{ this, &Isd4000App::callbackConnect };
		Slot<Device&> slotDisconnect{ this, &Isd4000App::callbackDisconnect };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortAdded{ this, &Isd4000App::callbackPortAdded };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortChanged{ this, &Isd4000App::callbackPortChanged };
		Slot<Device&, SysPort&> slotPortRemoved{ this, &Isd4000App::callbackPortRemoved };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortSelected{ this, &Isd4000App::callbackPortSelected };
		Slot<Device&, const deviceInfo_t&> slotDeviceInfo{ this, &Isd4000App::callbackDeviceInfo };
		Slot<Device&, const deviceStats_t&> slotPacketStats{ this, &Isd4000App::callbackPacketStats };
		Slot<Device&, const bootloaderProgress_t&> slotBootloader{ this, &Isd4000App::callbackBootloader };

		Slot<Ahrs&, uint64_t, const quaternion_t&, real_t, real_t> slotAhrsData{ this, &Isd4000App::callbackAhrs };
		Slot<GyroSensor&, const vector3_t&> slotGyroData{ this, &Isd4000App::callbackGyroData };
		Slot<AccelSensor&, const vector3_t&> slotAccelData{ this, &Isd4000App::callbackAccelData };
		Slot<MagSensor&, const vector3_t&> slotMagData{ this, &Isd4000App::callbackMagData };
		Slot<AccelSensor&, uint32_t, const vector3_t&, uint8_t> slotAccelCal{ this, &Isd4000App::callbackAccelCal };

		Slot<Isd4000&, uint64_t, real_t, real_t, real_t> slotPressure{ this, &Isd4000App::callbackPressureData };
		Slot<Isd4000&, real_t, real_t> slotTemperature{ this, &Isd4000App::callbackTemperatureData };
		Slot<Isd4000&> slotScriptDataReceived{ this, &Isd4000App::callbackScriptDataReceived };
		Slot<Isd4000&, bool_t> slotSettingsUpdated{ this, &Isd4000App::callbackSettingsUpdated };
		Slot<Isd4000&, const isd4000PressureCal_t&> slotPressureCalCert{ this, &Isd4000App::callbackPressureCal };
		Slot<Isd4000&, const isd4000TemperatureCal_t&> slotTemperatureCalCert{ this, &Isd4000App::callbackTemperatureCal };


	private:
		Isd4000* m_isd4000;
		std::string m_path;

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

		void callbackPressureData(Isd4000& isd4000, uint64_t timeUs, real_t pressureBar, real_t depthM, real_t pressureBarRaw);
		void callbackTemperatureData(Isd4000& isd4000, real_t temperatureC, real_t temperatureRawC);
		void callbackScriptDataReceived(Isd4000& isd4000);
		void callbackSettingsUpdated(Isd4000& isd4000, bool_t ok);
		void callbackPressureCal(Isd4000& isd4000, const isd4000PressureCal_t& cal);
		void callbackTemperatureCal(Isd4000& isd4000, const isd4000TemperatureCal_t& cal);
	};
}

//--------------------------------------------------------------------------------------------------
#endif

