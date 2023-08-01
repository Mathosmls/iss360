#ifndef ISLSDKDEVICE_H_
#define ISLSDKDEVICE_H_
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* @file This module contains definitions and functions that all Impact Subsea devices support. 
***************************************************************************************************/

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types -------------------------------------------

/// When a device event occurs that can be handled without knowing exactly what device it is a
/// callback function with the signature ::sdkEventHandler_t is called. 
///
/// To set this callback see islSdkInitialise(). 
/// The {@link sdkEventHandler_t eventId} parameter is set to one of these and this essentially defines how to cast the void* {@link sdkEventHandler_t data} parameter in the callback
typedef enum
{
	EVENT_DEVICE_ERROR,						///< An error occurred. Cast void* to sdkError_t *. { @link sdkError_t errorCode } will be one of ::deviceErrorCodes_t
	EVENT_DEVICE_CONNECTED,					///< A device has connected with the SDK. Cast void* to deviceInfo_t *
	EVENT_DEVICE_COMMS_RESEND,				///< A device has failed to acknowledge packets. void* is NULLPTR
	EVENT_DEVICE_DISCONNECTED,				///< A device has lost all connections with the SDK. void* is NULLPTR
	EVENT_DEVICE_INFO,						///< Device info message. Cast void* to deviceInfo_t *
	EVENT_DEVICE_DELETED,					///< The device object in the SDK has been deleted. void* is NULLPTR
	EVENT_DEVICE_CONNECTION_ADDED,			///< The SDK has added a new means of communicating to the device. Cast void* to portConnectionInfo_t *
	EVENT_DEVICE_CONNECTION_CHANGED,		///< Some of the connection parameters to the device have changed, e.g baudrate. Cast void* to portConnectionInfo_t *
	EVENT_DEVICE_CONNECTION_REMOVED,		///< The connection to the device has been removed due to loss of comms. Cast void* to uint32_t * to get connection ID
	EVENT_DEVICE_CONNECTION_SELECTED,		///< This connection is currently being use to communicate with the device. Cast void* to portConnectionInfo_t *
	EVENT_DEVICE_STATS,						///< This event is generated at 1Hz and sends information about device packet counts. Cast void* to deviceStats_t *.
	EVENT_DEVICE_BOOTLOADER_PROGRESS,		///< When a device's firmware is updating these events will appear. Cast void* to bootloaderProgress_t *
} deviceEvent_t;

/// Error codes see ::EVENT_DEVICE_ERROR.
typedef enum
{
	ERROR_DEVICE_ID_INVALID,				///< No device object exists with this id
} deviceErrorCodes_t;

/// Every device type has a product id.
typedef enum
{
	PID_UNKNOWN = 0,
	PID_ISA500v1 = 1242,					///< Obsolete product id of the altimeter
	PID_ISD4000v1 = 1299,					///< Obsolete product id of the depth sensor
	PID_ISM3Dv1 = 1444,						///< Obsolete product id of the high grade heading, pitch, roll sensor
	PID_ISS360v1 = 1660,					///< Obsolete product id of the sonar
	PID_ISA500 = 1336,						///< Product id of the altimeter
	PID_ISD4000 = 1342,						///< Product id of the depth sensor
	PID_ISM3D = 1461,						///< Product id of the high grade heading, pitch, roll sensor
	PID_ISS360 = 1791,						///< Product id for the sonar
	PID_TESTJIG = 1384,						///< Factory use
    PID_SOL4 = 1865,						///< Factory use
	PID_ANY = 0xffff,						///< When the product id is used as a filter this allows all
} devicePid_t;

/// If a device has a serial port then it may support some or all of theses.
typedef enum
{
	UART_MODE_RS232,						///< Device serial port uses RS232
	UART_MODE_RS485,						///< Device serial port uses RS485
	UART_MODE_RS485_TERM,					///< Device serial port uses RS485 with 120 ohm termination resistor
	UART_MODE_UNKNOWN = -1,
} deviceUartMode_t;

/// If a device has a ethernet port then it may support some or all of theses.
typedef enum
{
	PHY_MDIX_NORMAL,						///< Ethernet RX/TX pairs are not swapped in the device PHY
	PHY_MDIX_SWAP,							///< Ethernet RX/TX pairs are swapped in the device PHY
	PHY_MDIX_AUTO,							///< Ethernet RX/TX pairs are automatically swapped in the device PHY if needed
	PHY_MDIX_UNKNOWN = -1,
} devicePhyMdixMode_t;

/// If a device has a ethernet port then it may support some or all of theses.
typedef enum
{
	PHY_PORT_AUTO,							///< Device ethernet auto negotiates to determine speed and duplex 
	PHY_PORT_10BASETXHALF,					///< Device ethernet is forced to run at 10-BASE half duplex
	PHY_PORT_10BASETXFULL,					///< Device ethernet is forced to run at 10-BASE full duplex
	PHY_PORT_100BASETXHALF,					///< Device ethernet is forced to run at 100-BASE half duplex
	PHY_PORT_100BASETXFULL,					///< Device ethernet is forced to run at 100-BASE full duplex
	PHY_PORT_UNKNOWN = -1,
} devicePhyPortMode_t;

typedef struct
{
	bool_t enable;
	uint8_t length;
	uint8_t str[32];
} deviceCustomStr_t;

typedef struct
{
	uint32_t number;						///< Script number
    uint32_t nameSize;                      ///< Length of name with null terminater
    uint32_t codeSize;                      ///< Length of code with null terminater
	const uint8_t* name;					///< Script name
	const uint8_t* code;					///< Script code
} deviceScript_t;

typedef struct
{
	uint8_t type;							///< Script variable type
	const uint8_t* name;					///< Script variable name
	const uint8_t* description;				///< Script variable description
} scriptVarInfo_t;

typedef struct
{
	uint32_t count;							/// Number of items in the var list
	scriptVarInfo_t* var;					/// List of the script varibles
} deviceScriptVars_t;

typedef struct
{
	uint32_t listId;						///< Id of the list
	uint32_t count;							///< Number of elements in the names array
	uint8_t** names;						///< Poniter to array of pointers to names of string
} deviceOutputStringNames_t;

/// Information that defines a device
typedef struct
{
	uint32_t pn;							///< Part number
	uint32_t sn;							///< Serial number
	devicePid_t pid;						///< Product id
	uint8_t config;							///< configuration value
	uint16_t status;						///< status value
	uint32_t firmwareBuildNum;				///< Firmware build number
	uint16_t firmwareVersionBcd;			///< Firmware revision. Each nibble represents a digit, e.g 0x1234 would be V1.2.3.4.
	bool_t bootloaderMode;					///< If true device is in bootloader mode
	bool_t inUse;							///< If true device is also connected to another host
} deviceInfo_t;

/// Device metrics for packets. This event is generated once per second
typedef struct
{
	uint32_t txPacketCount;					///< Number of send packets since a connection
	uint32_t rxPacketCount;					///< Number of received packets since a connection
	uint32_t resendPacketCount;				///< Number of re-sent packets since a connection
} deviceStats_t;

typedef enum
{
	BOOTLDR_INVOKING,					///< The bootloader is being invoked
	BOOTLDR_UPLOADING,					///< Firmware is uploading to the device
	BOOTLDR_BOOTING_APP,				///< Firmware upload complete.
	BOOTLDR_ERR_WRONG_PID,				///< Incorrect firmware for the device
	BOOTLDR_ERR_BUSY,					///< Device is already loading firmware
	BOOTLDR_ERR_FAILED,					///< An error occured, maybe loss of comms
	BOOTLDR_ERR_CANCELED,				///< Firmware uploading was cancled by the user or SDK
} bootloaderStatus_t;

/// Holds information about the progress of a firmware update
typedef struct
{
	uint32_t totalSize;						///< Total size in bytes of the firmware to load
	uint32_t loadedSize;					///< Number of bytes loaded so far
	bootloaderStatus_t status;				///< Fimrware upload status
} bootloaderProgress_t;

//---------------------------------- Public Function Prototypes ------------------------------------

/***************************************************************************************************
* Delete a device within the SDK. This should be done when the application no longer needs to talk to a device.
*
* @param id The id of the device as returned by the SDK
***************************************************************************************************/
void islSdkDeviceDelete(uint32_t id);

/***************************************************************************************************
* Sends a reset command to a connected device.
*
* @param id The id of the device as returned by the SDK
***************************************************************************************************/
void islSdkDeviceReset(uint32_t id);

/***************************************************************************************************
* Connects to a device.
*
* @param id The id of the device as returned by the SDK
***************************************************************************************************/
void islSdkDeviceConnect(uint32_t id);

/***************************************************************************************************
* Requests the deviceInfo_t struct from a connected device.
* This will trigger a device event with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_DEVICE_INFO if the info has changed
*
* @param id The id of the device as returned by the SDK
* @note to set the device event callback see islSdkInitialise()
***************************************************************************************************/
void islSdkDeviceGetInfo(uint32_t id);

/***************************************************************************************************
* Force a device to use the specified connection.
* If a device has multiple connections, e.g RS232 and ethernet, then this function can be used to switch
* between them. The SDK will automatically switch to the fastest connection as connections are discovered
* (by calling islSdkPortAutoDiscoverDevices()) or fail.
* If the connection is changed a device event with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_DEVICE_CONNECTION_CHANGED will occur

* @param id The id of the device as returned by the SDK
* @param portId The id of the port as returned by the SDK
* @note to set the device event callback see islSdkInitialise()
***************************************************************************************************/
void islSdkDeviceSetConnection(uint32_t id, uint32_t portId);

/***************************************************************************************************
* Load firmware onto a device.
* A device event will be trigger as a result with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_DEVICE_BOOTLOADER_PROGRESS
*
* @param id The id of the device as returned by the SDK
* @param fwId The id of the firmware as returned by the SDK
* @return
*		 - TRUE if #fwId is valid
*		 - FALSE if #fwId is incorrect
* @note to set the device event callback see islSdkInitialise()
***************************************************************************************************/
bool_t islSdkDeviceProgramFirmware(uint32_t id, uint32_t fwId);

/***************************************************************************************************
* Cancel the firmware programming to a device.
* If the programing of firmware is terminated in any way then the unit will reside in bootloader mode until firmware is loaded.
* A device events will be trigger as a result with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_DEVICE_BOOTLOADER_PROGRESS
*
* @param deviceId The id of the device as returned by the SDK
* @note to set the device event callback see islSdkInitialise()
***************************************************************************************************/
void islSdkDeviceCancelFirmware(uint32_t deviceId);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
