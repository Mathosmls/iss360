#ifndef ISA500APP_H_
#define ISA500APP_H_

//------------------------------------------ Includes ----------------------------------------------

#include "isa500.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Isa500App
	{
	public:
		Isa500App(void);
		~Isa500App(void);
		void disconnectAllEvents(void);
		void setDevice(Isa500* device);
		void doTask(int32_t key, const str_t* path);

		Slot<Device&, uint32_t, const utf8_t*> slotError{ this, &Isa500App::callbackError };
		Slot<Device&> slotDelete{ this, &Isa500App::callbackDeleteted };
		Slot<Device&> slotConnect{ this, &Isa500App::callbackConnect };
		Slot<Device&> slotDisconnect{ this, &Isa500App::callbackDisconnect };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortAdded{ this, &Isa500App::callbackPortAdded };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortChanged{ this, &Isa500App::callbackPortChanged };
		Slot<Device&, SysPort&> slotPortRemoved{ this, &Isa500App::callbackPortRemoved };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortSelected{ this, &Isa500App::callbackPortSelected };
		Slot<Device&, const deviceInfo_t&> slotDeviceInfo{ this, &Isa500App::callbackDeviceInfo };
		Slot<Device&, const deviceStats_t&> slotPacketStats{ this, &Isa500App::callbackPacketStats };
		Slot<Device&, const bootloaderProgress_t&> slotBootloader{ this, &Isa500App::callbackBootloader };

		Slot<Ahrs&, uint64_t, const quaternion_t&, real_t, real_t> slotAhrsData{ this, &Isa500App::callbackAhrs };
		Slot<GyroSensor&, const vector3_t&> slotGyroData{ this, &Isa500App::callbackGyroData };
		Slot<AccelSensor&, const vector3_t&> slotAccelData{ this, &Isa500App::callbackAccelData };
		Slot<MagSensor&, const vector3_t&> slotMagData{ this, &Isa500App::callbackMagData };
		Slot<AccelSensor&, uint32_t, const vector3_t&, uint8_t> slotAccelCal{ this, &Isa500App::callbackAccelCal };

		Slot<Isa500&, const isa500Echoes_t&> slotEchoData{ this, &Isa500App::callbackEchoData };
		Slot<Isa500&, real_t> slotTemperatureData{ this, &Isa500App::callbackTemperatureData };
		Slot<Isa500&, real_t> slotVoltageData{ this, &Isa500App::callbackVoltageData };
		Slot<Isa500&, bool_t> slotTriggerData{ this, &Isa500App::callbackTriggerData };
		Slot<Isa500&> slotScriptDataReceived{ this, &Isa500App::callbackScriptDataReceived };
		Slot<Isa500&, bool_t> slotSettingsUpdated{ this, &Isa500App::callbackSettingsUpdated };
		Slot<Isa500&, const isa500TestWaveform_t&> slotPingTestData{ this, &Isa500App::callbackPingTestData };


	private:
		Isa500* m_isa500;

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

		void callbackEchoData(Isa500& isa500, const isa500Echoes_t& echoes);
		void callbackTemperatureData(Isa500& isa500, real_t temperatureC);
		void callbackVoltageData(Isa500& isa500, real_t voltage12);
		void callbackTriggerData(Isa500& isa500, bool_t risingEdge);
		void callbackScriptDataReceived(Isa500& isa500);
		void callbackSettingsUpdated(Isa500& isa500, bool_t ok);
		void callbackPingTestData(Isa500& isa500, const isa500TestWaveform_t& data);
	};
}

//--------------------------------------------------------------------------------------------------
#endif