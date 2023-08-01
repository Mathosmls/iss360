#ifndef ISLSDKPORT_H_
#define ISLSDKPORT_H_
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* @file This module contains definitions and functions for managing serial and network ports
***************************************************************************************************/

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "islSdkDevice.h"
#include "platform/uart.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types -------------------------------------------

/// When a port event occurs a callback function with the signature ::sdkEventHandler_t is called. 
/// To set this callback see islSdkInitialise(). 
/// The {@link sdkEventHandler_t id} parameter is set to the SDK assigned id of the port and the
/// {@link sdkEventHandler_t eventId} parameter is set to one of these and this essentially defines how to cast the void* {@link sdkEventHandler_t data} parameter in the callback
typedef enum
{
	EVENT_PORT_ERROR,						///< An error occurred. Cast void* to sdkError_t *. { @link sdkError_t errorCode } will be one of ::portErrorCodes_t
	EVENT_PORT_CREATED,						///< A communication port on the system has been found and created. Cast void* to portInfo_t *
	EVENT_PORT_DELETED,						///< A communication port on the system is no longer available. void* is NULLPTR
	EVENT_PORT_OPEN,						///< The port has been opened by the SDK. void* is NULLPTR
	EVENT_PORT_OPEN_FAILED,					///< The port failed to open. void* is NULLPTR
	EVENT_PORT_CLOSE,						///< The port has been closed. void* is NULLPTR
	EVENT_PORT_STATS,						///< Reports the tx, rx byte count and packet count for the port. Cast void* to portStats_t *
	EVENT_PORT_AUTO_DISCOVERY_STARTED,		///< Auto discovery has started on this port. Cast void* to autoDiscoveryType_t *
	EVENT_PORT_AUTO_DISCOVERY_EVENT,		///< A discovery packet has been sent from this port. Cast void* to autoDiscoveryInfo_t *
	EVENT_PORT_AUTO_DISCOVERY_FINISHED,		///< Auto discovery has stopped on this port. Cast void* to autoDiscoveryInfo_t *
	EVENT_PORT_DEVICE_DISCOVERED,			///< A device has been discovered on this port. Cast void* to portDeviceDiscovery_t *
	EVENT_PORT_NMEA_STRING,					///< A new NMEA string has been received on this port. Cast void* to portNmeaString_t *
    EVENT_PORT_DATA,                        ///< Cast void* to portData_t *
} portEvent_t;

/// Connection information for a port. Check the port type to know which elements are valid.
typedef struct
{
	uint32_t baudrate;						///< Baudrate of the serial port
	uint32_t ipAddress;						///< IpAddress data is sent to
	uint16_t port;							///< The port data is sent to
} connectionMeta_t;

/// Error codes see ::EVENT_PORT_ERROR.
typedef enum
{
	ERROR_PORT_ID_INVALID,					///< No port object exists with this id
	ERROR_PORT_ID_NOT_SOL,
	ERROR_PORT_SOL_SET,
} portErrorCodes_t;

/// The type of port
typedef enum
{
	PORT_SERIAL,							///< A serial port
	PORT_NETWORK,							///< A network port
	PORT_SOL,								///< A Serial Over Lan port
} portType_t;

/// Infomation about a port
typedef struct
{
	portType_t type;						///< Type of port
	str_t* name;							///< Pointer to the UTF8 name of the port
} portInfo_t;

/// Infomation about a port's metrics. This event is generated every seconds
typedef struct
{
	uint32_t rxCount;						///< Number of received bytes since the last event of this type.
	uint32_t txCount;						///< Number of transmited bytes since the last event of this type.
	uint32_t txPacketCount;					///< Number of packets sent since the last event of this type.
	uint32_t rxPacketCount;					///< Number of packets recevied since the last event of this type.
	uint32_t badRxPacketCount;				///< Number of recevied malformed packets since the last event of this type.
} portStats_t;

/// Information about a devices connection
typedef struct
{
	uint32_t id;							///< SDK assigned ID of the port
	portType_t type;						///< Type of port
	connectionMeta_t meta;					///< Connection details such as baudrate or IP address
} portConnectionInfo_t;

/// Information when a device is discovered 
typedef struct
{
	uint32_t deviceId;						///< SDK assigned id for this device
	deviceInfo_t info;						///< device information
} portDeviceDiscovery_t;

/// Nmea string event 
typedef struct
{
	bool_t checkSumOk;						///< True if the strings checksum matches the computed checksum
	uint32_t size;							///< length of the string
	uint8_t* str;							///< Pointer to the NULL terminated string
} portNmeaString_t;

typedef enum
{
	AUTODISCOVERY_ISLDEVICE,
	AUTODISCOVERY_NMEA,
} autoDiscoveryType_t;

typedef struct
{
	portConnectionInfo_t port;				///< port information
	autoDiscoveryType_t autoDiscoveryType;	///< The type of auto discovery
	uint32_t discoveryCount;				///< Number of devices discovered so far 
    uint32_t deviceCount;                   ///< Number of devices using this port
	bool_t wasCancelled;					///< True if the discovery didn't finish
} autoDiscoveryInfo_t;

typedef struct
{
    uint32_t size;
    uint8_t* data;
} portData_t;

//----------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Starts searching for devices connected to the system over network ports.
* Example usage:
* @code islSdkPortLanAutoDiscoverDevices(0, 0, PID_ANY, 0xffffffff, 500); @endcode
*
* @param pn The part number of the device to find. 0 and 0xffff mean all part numbers
* @param sn The serial number of the device to find. 0 and 0xffff mean all serial numbers
* @param pid The product type to search for
* @param ipAddress IPv4 address to search on (big endian format eg 192.168.0.1 == 0x0100A8C0)
* @param port UDP port number to search on
* @param timeoutMs the number of milliseconds to listen for data before timing out and trying different port settings.
* This value is limited between 100 and 10000. Any value outside this range will be clipped
*
* @note An ::EVENT_PORT_DEVICE_DISCOVERED event will trigged for every discovery of a device on a port
* - If \p ipAddress is 0 then the gloabl broadcast IPv4 address 255.255.255.255 will be used
* - If \p port is 0 then the standard port 33005 will be used
***************************************************************************************************/
void islSdkPortLanAutoDiscoverDevices(uint32_t pn, uint32_t sn, devicePid_t pid, uint32_t ipAddress, uint16_t port, uint32_t timeoutMs);

/***************************************************************************************************
* Starts searching for devices connected to the system over serial and serial over lan.
* Example usage:
* @code islSdkPortSerialAutoDiscoverDevices(0, 0, 0, PID_ANY, 9600, 250); @endcode
* @param portId The id of the port to start auto discovery on. If this value is zero then auto discovery will happen on all serial port
* @param pn The part number of the device to find. 0 and 0xffff mean all part numbers
* @param sn The serial number of the device to find. 0 and 0xffff mean all serial numbers
* @param pid The product type to search for
* @param baudrate The baudrate to use
* @param timeoutMs The number of milliseconds to listen for data before timing out and trying different port settings.
* This value is limited between 100 and 10000. Any value outside this range will be clipped.
*
* @note An ::EVENT_PORT_DEVICE_DISCOVERED event will trigged for every discovery of a device on a port
* - If \p portId is 0 then all available serial ports will be opened
***************************************************************************************************/
void islSdkPortSerialAutoDiscoverDevices(uint32_t portId, uint32_t pn, uint32_t sn, devicePid_t pid, uint32_t baudrate, uint32_t timeoutMs);

/***************************************************************************************************
* Starts searching for a NMEA strings connected to the system over a serial ports and serial over lan.
* Example usage:
* @code islSdkPortSerialAutoDiscoverNmeaDevices(0, 9600, 1200); @endcode
* @param portId The id of the port to start auto discovery on. If this value is zero then auto discovery will happen on all serial port
* @param baudrate The baudrate to use
* @param timeoutMs the number of milliseconds to listen for data before timing out and trying different port settings.
* This value is limited between 100 and 10000. Any value outside this range will be clipped
*
* @note An ::EVENT_PORT_NMEA_STRING event will be triggered for every reception of a NMEA string on a port from now on.
* - If \p portId is 0 then all available serial ports will be opened
***************************************************************************************************/
void islSdkPortSerialAutoDiscoverNmeaDevices(uint32_t portId, uint32_t baudrate, uint32_t timeoutMs);

/***************************************************************************************************
* Stop auto discovery on a port
* @param portId The SDK assigned id of the port. If this parameter is zero then auto discovery will be stopped on all ports
***************************************************************************************************/
void islSdkPortAutoDiscoverStop(uint32_t portId);

/***************************************************************************************************
* Open a port
* @param portId The SDK assigned id of the port.
***************************************************************************************************/
void islSdkPortOpen(uint32_t portId);

/***************************************************************************************************
* Write data to a port
* @param portId The SDK assigned id of the port.
* @param data The data to write
* @param size The number of bytes data points to
* @param meta The baudrate to send the data at or ip address / port to send the data to
***************************************************************************************************/
void islSdkPortWrite(uint32_t portId, const uint8_t* data, uint32_t size, const connectionMeta_t* meta);

/***************************************************************************************************
* Enable or disable port data events
* @param portId The SDK assigned id of the port.
* @param onOff True enables events
***************************************************************************************************/
void islSdkPortDataEventEnable(uint32_t portId, bool_t onOff);

/***************************************************************************************************
* Close a port.
* @param portId The SDK assigned id of the port
* @note An ::EVENT_PORT_CLOSE event will trigged if the port closed
***************************************************************************************************/
void islSdkPortClose(uint32_t portId);

/***************************************************************************************************
* Configure the serial properties of a serial port or Serial Over Lan (SOL) port.
* @param portId The SDK assigned id of the port
* @param baudrate The baudrate to set the serial port to
* @param dataBits The number of data bits 5,6,7,8
* @param parity The parity
* @param stopBits The number of stop bits
* @warning For SOL ports this will only work if the port is configured with useTelnetRfc2217 set true.
***************************************************************************************************/
void islSdkPortConfigureSerial(uint32_t portId, uint32_t baudrate, uint8_t dataBits, uartParity_t parity, uartStopBits_t stopBits);

/***************************************************************************************************
* Create a Serial Over Lan (SOL) port.
* @param isTcp
			- TRUE if the connection is to be TCP client
			- FALSE if the connection is to be UDP client
* @param useTelnetRfc2217
			- TRUE if the connection should use the telent RFC2217 protocol. This will allow configuration of the serial port
			- FALSE if the connection is RAW. The serial data is placed in the payload of the TCP/UDP packet
* @param ipAddress	The IP address of the SOL server to connect to, big endian format eg 192.168.0.1 == 0x0100A8C0. Set to zero if not known yet
* @param port		The port number of the SOL server to connect to. Set to zero if not known yet
* @note An ::EVENT_PORT_CREATED event will trigger on creation of the port
***************************************************************************************************/
void islSdkPortSolCreate(bool_t isTcp, bool_t useTelnetRfc2217, uint32_t ipAddress, uint16_t port);

/***************************************************************************************************
* Configure a Serial Over Lan (SOL) port.
* @param portId The SDK assigned id of the port
* @param isTcp 
			- TRUE if the connection is to be TCP client
			- FALSE if the connection is to be UDP client
* @param useTelnetRfc2217 
			- TRUE if the connection should use the telent RFC2217 protocol. This will allow configuration of the serial port
			- FALSE if the connection is RAW. The serial data is placed in the payload of the TCP/UDP packet
* @param ipAddress	The IP address of the SOL server to connect to, big endian format eg 192.168.0.1 == 0x0100A8C0.
* @param port		The port number of the SOL server to connect to
* @note This function can be called on an open port to reconfigure it
***************************************************************************************************/
void islSdkPortSolConfigure(uint32_t portId, bool_t isTcp, bool_t useTelnetRfc2217, uint32_t ipAddress, uint16_t port);

/***************************************************************************************************
* Delete a Serial Over Lan (SOL) port.
* @param portId The SDK assigned id of the port
* @note An ::EVENT_PORT_DELETED event will trigged if the port closed
***************************************************************************************************/
void islSdkPortSolDelete(uint32_t portId);

#ifdef __cplusplus
}
#endif
#endif
