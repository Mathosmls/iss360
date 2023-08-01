#ifndef SYSPORT_H_
#define SYSPORT_H_

//------------------------------------------ Includes ----------------------------------------------

#include "islSdkPort.h"
#include "islSdkDevice.h"
#include <string>
#include "sigSlot.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class SysPort
	{
	private:
		uint32_t m_id;
		std::string m_name;
		portType_t m_type;
		bool_t m_isOpen;
		bool_t m_isAutoDiscovering;

		void signalSubscribersChanged(uint32_t subscriberCount);

	public:

		const uint32_t& id = m_id;
		const std::string& name = m_name;
		const portType_t& type = m_type;
		const bool_t& isOpen = m_isOpen;
		const bool_t& isAutoDiscovering = m_isAutoDiscovering;

		Signal<SysPort&, uint32_t, const utf8_t*> onError;
		Signal<SysPort&> onDelete;
		Signal<SysPort&, bool_t> onOpen;
		Signal<SysPort&> onClose;
		Signal<SysPort&, const portStats_t&> onPortStats;
		Signal<SysPort&, autoDiscoveryType_t> onDiscoveryStarted;
		Signal<SysPort&, const autoDiscoveryInfo_t&> onDiscoveryEvent;
		Signal<SysPort&, const autoDiscoveryInfo_t&> onDiscoveryFinished;
		Signal<SysPort&, uint32_t, const deviceInfo_t&> onDeviceDiscovered;
		Signal<SysPort&, const uint8_t*, uint32_t, bool_t> onNmeaString;
		Signal<SysPort&, const uint8_t*, uint32_t> onData{ this, &SysPort::signalSubscribersChanged };

		static const std::vector<SysPort*>& getList();
		static SysPort* findById(uint32_t id);
		static void deleteAll();
		static void createSol(bool_t useTcp, bool_t useTelnetRfc2217, uint32_t ipAddress, uint16_t port);
		SysPort(uint32_t id, portInfo_t& info);
		~SysPort();
		void handleEvent(uint64_t timeMs, uint32_t eventId, const void* data);
		void stopDiscovery();
		void startDiscovery(uint32_t pn, uint32_t sn, devicePid_t pid, const uint32_t* baudrateList, uint32_t baudCount, uint32_t timeoutMs);
		void startDiscovery(uint32_t pn, uint32_t sn, devicePid_t pid, uint32_t ipAddress, uint16_t port, uint32_t timeoutMs);
		void startNmeaDiscovery(const uint32_t* baudrateList, uint32_t baudCount, uint32_t timeoutMs);
		void open();
		void close();
		void write(uint8_t* data, uint32_t size, connectionMeta_t& meta);
		void config(uint32_t baudrate, uint8_t dataBits, uartParity_t parity, uartStopBits_t stopBits);
	};
}

//--------------------------------------------------------------------------------------------------

#endif
