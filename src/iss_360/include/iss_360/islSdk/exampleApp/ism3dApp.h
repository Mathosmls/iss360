#ifndef ISM3DAPP_H_
#define ISM3DAPP_H_

//------------------------------------------ Includes ----------------------------------------------

#include "ism3d.h"
#include "islSdkVector.h"
#include "helpers/ahrsCal.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class Ism3dApp
	{
	public:
		Ism3dApp(void);
		~Ism3dApp(void);
		void disconnectAllEvents(void);
		void setDevice(Ism3d* device);
		void doTask(int32_t key, const str_t* path);

		Slot<Device&, uint32_t, const utf8_t*> slotError{ this, &Ism3dApp::callbackError };
		Slot<Device&> slotDelete{ this, &Ism3dApp::callbackDeleteted };
		Slot<Device&> slotConnect{ this, &Ism3dApp::callbackConnect };
		Slot<Device&> slotDisconnect{ this, &Ism3dApp::callbackDisconnect };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortAdded{ this, &Ism3dApp::callbackPortAdded };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortChanged{ this, &Ism3dApp::callbackPortChanged };
		Slot<Device&, SysPort&> slotPortRemoved{ this, &Ism3dApp::callbackPortRemoved };
		Slot<Device&, SysPort&, const connectionMeta_t&> slotPortSelected{ this, &Ism3dApp::callbackPortSelected };
		Slot<Device&, const deviceInfo_t&> slotDeviceInfo{ this, &Ism3dApp::callbackDeviceInfo };
		Slot<Device&, const deviceStats_t&> slotPacketStats{ this, &Ism3dApp::callbackPacketStats };
		Slot<Device&, const bootloaderProgress_t&> slotBootloader{ this, &Ism3dApp::callbackBootloader };

		Slot<Ahrs&, uint64_t, const quaternion_t&, real_t, real_t> slotAhrsData{ this, &Ism3dApp::callbackAhrs };
		Slot<GyroSensor&, const vector3_t&> slotGyroData{ this, &Ism3dApp::callbackGyroData };
		Slot<AccelSensor&, const vector3_t&> slotAccelData{ this, &Ism3dApp::callbackAccelData };
		Slot<MagSensor&, const vector3_t&> slotMagData{ this, &Ism3dApp::callbackMagData };
		Slot<AccelSensor&, uint32_t, const vector3_t&, uint8_t> slotAccelCal{ this, &Ism3dApp::callbackAccelCal };

		Slot<Ism3d&> slotScriptDataReceived{ this, &Ism3dApp::callbackScriptDataReceived };	
		Slot<Ism3d&, bool_t> slotSettingsUpdated{ this, &Ism3dApp::callbackSettingsUpdated };

	private:
		Ism3d* m_ism3d;

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

		void callbackScriptDataReceived(Ism3d& isa500);
		void callbackSettingsUpdated(Ism3d& isa500, bool_t ok);
	};
}

//--------------------------------------------------------------------------------------------------
#endif