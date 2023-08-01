#ifndef ISLSDK_H_
#define ISLSDK_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "islSdkPort.h"
#include "islSdkFirmware.h"
#include "islSdkLogging.h"
#include "islSdkVector.h"
#include "islSdkDevice.h"
#include "islSdkAhrs.h"
#include "islSdkIsa500.h"
#include "islSdkIsd4000.h"
#include "islSdkIsm3d.h"
#include "islSdkIss360.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types -------------------------------------------

//---------------------------------- Public Function Prototypes ------------------------------------

/***************************************************************************************************
* Initialises and starts the SDK thread.
*
* @param newEvent A pointer to a function that will be called every time the event queue transitions to not empty
* @param portEvent A pointer to a function that will be called when there is a port event
* @param deviceEvent A pointer to a function that will be called when there is a generic device event
* @return
*		 - TRUE if succeeded
*		 - FALSE if already called or a parameter was NULL
* @note This function must be called before any other SDK function with the exception of islSdkVersion()
* @warning Platform function pointers must be set before calling this function
***************************************************************************************************/
bool_t islSdkInitialise(void(*newEvent)(void), sdkEventHandler_t portEvent, sdkEventHandler_t deviceEvent);

/***************************************************************************************************
* Run all the SDK tasks.
* Just about all SDK functionality happens under this function call. Here is a basic list of the SDK tasks.
* 1.	Manages serial and network ports
* 2.	Processes and manage device tasks and auto discovery
* 3.	Manage logging playback
* 4.	Place SDK events on the event queue
* 5.	Process user requests from the command queue
* This is a non blocking function that should be called periodically, every 50 milliseconds is ideal.
***************************************************************************************************/
void islSdkRun(void);

/***************************************************************************************************
* Triggers the processing of the event queue.
* Ideally this should be called from the thread that will handle all SDK events to avoid thread
* synchronisation issues. It can either be frequently called or in responce to the newEvent callback,
* (the first parameter passed to islSdkInitialise()). This function pops messages off the SDK's event
* queue and calls the appropriate callbacks.
***************************************************************************************************/
void islSdkFireEvents(void);

/***************************************************************************************************
* Shuts down the SDK.
* This function closes all resources and frees all memory.
***************************************************************************************************/
void islSdkExit(void);

/***************************************************************************************************
* Convert the event SDK millsecond timeMs parameter to the number of milliseconds since the Unix epoch.
* @return Number of milliseconds since the Unix epoch - 00:00:00 UTC on 1 January 1970
***************************************************************************************************/
uint64_t islSdkGetEpochMsTime(uint64_t sdkTimeMs);

/***************************************************************************************************
* Get the version of SDK.
* @return The version in BCD (Binary Coded Decimal) format. Each nibble represents a digit eg 0x1234 = V1.2.3.4
***************************************************************************************************/
uint16_t islSdkVersion(void);

/***************************************************************************************************
* sets the number of retries and interval to seach for devices once comms are lost
*
* @param count The number of retries
* @param timeoutMs The number of milliseconds to wait for a reply
* 
* ***************************************************************************************************/
void islSdkSetDeviceRediscoveryTimings(uint32_t count, uint32_t timeoutMs);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
