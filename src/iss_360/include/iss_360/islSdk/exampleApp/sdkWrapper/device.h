#ifndef DEVICE_H_
#define DEVICE_H_

//------------------------------------------ Includes ----------------------------------------------

#include "islSdkDevice.h"
#include "sysPort.h"
#include "logger.h"
#include "sigSlot.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	struct deviceConnection_t
	{
		deviceConnection_t(SysPort* sysPort, connectionMeta_t meta) : sysPort(sysPort), meta(meta) {}
		SysPort* sysPort;
		connectionMeta_t meta;
	};

	enum class VarType { Byte, Int, Real, ByteArray, IntArray, RealArray };

	struct ScriptVar
	{
		const VarType type;
		const std::string name;
		const std::string description;
		ScriptVar(const scriptVarInfo_t* var) : type(static_cast<VarType>(var->type)), name{ (const char*)var->name }, description{ (const char*)var->description } {};
	};

	enum class DataState { Invalid, Pending, Valid };

	struct DeviceScript
	{
		DataState state;
		std::string name;
		std::string code;
		DeviceScript() : state(DataState::Invalid) {};
		DeviceScript(const deviceScript_t* script) : state(DataState::Valid), name{ (const char*)script->name }, code{ (const char*)script->code } {};
	};

	class Device
	{
	public:
		const uint32_t& id = m_id;
		const bool_t& isConnected = m_isConnected;
		const uint32_t& reconnectCount = m_reconnectCount;
		const deviceInfo_t& info = m_info;
		const std::vector<deviceConnection_t>& connections = m_connectionList;
		
		Signal<Device&, uint32_t, const utf8_t*> onError;
		Signal<Device&> onDelete;
		Signal<Device&> onConnect;
		Signal<Device&> onDisconnect;
		Signal<Device&, SysPort&, const connectionMeta_t&> onPortAdded;
		Signal<Device&, SysPort&, const connectionMeta_t&> onPortChanged;
		Signal<Device&, SysPort&> onPortRemoved;
		Signal<Device&, SysPort&, const connectionMeta_t&> onPortSelected;
		Signal<Device&, const deviceInfo_t&> onInfoChanged;
		Signal<Device&, const deviceStats_t&> onPacketStats;
		Signal<Device&, const bootloaderProgress_t&> onBootloader;

		static const std::vector<Device*>& getList();
		static Device* findById(uint32_t id);
		static void deleteAll();
		static void removeLoggerInst(const Logger& logger);
		Device(uint32_t id, const deviceInfo_t& info);
		virtual ~Device();
		void connect();
		void reset();
		void setConnection(const SysPort& port);
		void uploadFirmware(uint32_t fwId);
		void cancelFirmwareUpload();
		void handleBaseEvent(uint64_t timeMs, uint32_t eventId, const void* data);
		void setLogger(Logger* logger, uint32_t appId);
		virtual bool_t saveConfig(const str_t* fileName) {return false;}
		const deviceConnection_t* getConnection() { return m_connection; }

	protected:
		virtual void connectionEvent(bool_t connected);
		virtual void startStopLogging(bool_t start);
		struct
		{
			Logger* logger;
			uint32_t trackId;
			bool_t active;
		} m_log;
		deviceConnection_t* m_connection;
		std::vector<deviceConnection_t> m_connectionList;

	private:
		uint32_t m_id;
		deviceInfo_t m_info;
		bool_t m_isConnected;
		uint32_t m_reconnectCount;
		deviceConnection_t* findDeviceConnection(uint32_t sysPortId);
		Slot<Logger&, const utf8_t*> slotOnNewLogFile{ this, &Device::callbackNewLogFile };
		void callbackNewLogFile(Logger& logger, const utf8_t* filename);
	};
}

//--------------------------------------------------------------------------------------------------

#endif
