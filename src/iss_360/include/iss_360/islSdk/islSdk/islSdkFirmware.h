#ifndef FIRMWAREMGR_H_
#define FIRMWAREMGR_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* @file This module contains definitions and functions for loading firmware file (.fwi)
***************************************************************************************************/

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "islSdkDevice.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

/// Information about a loaded firmware object
typedef struct
{
	uint32_t id;					///< Id of the firmware object as returned by islSdkFirmwareLoadFile()
	devicePid_t pid;				///< Product id the firmware is for
	uint32_t size;					///< Size of the firmware in bytes
	uint16_t versionBCD;			///< Firmware version in BCD (Binary Coded Decimal) format. Each nibble represents a digit eg 0x1234 = V1.2.3.4
} firmwareInfo_t;

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Loads a .fwi firmware file into the SDK
*
* @param fileName the file to open and load
* @return Zero if the file couldn't be loaded or the id of the firmware object
***************************************************************************************************/
uint32_t islSdkFirmwareLoadFile(const str_t* fileName);

/***************************************************************************************************
* Deletes a loaded firmware object
*
* @param id Id of the firmware object as returnd by islSdkFirmwareLoadFile()
* @return
*		 - TRUE if succeeded
*		 - FALSE if the firmware object is in use or the id is invalid
***************************************************************************************************/
bool_t islSdkFirmwareDelete(uint32_t id);

/***************************************************************************************************
* Checks if a firmware object is in use
*
* @param id Id of the firmware object as returnd by islSdkFirmwareLoadFile()
* @return
*		 - TRUE if in use
*		 - FALSE if free or invalid id
***************************************************************************************************/
bool_t islSdkFirmwareInUse(uint32_t id);

/***************************************************************************************************
* Gets the firmwareInfo_t information for a firmware item
*
* @param id Id of the firmware object as returnd by islSdkFirmwareLoadFile()
* @param info A pointer to a firmwareInfo_t struct which will be populated
* @return
*		 - TRUE if info was populated
*		 - FALSE if not firmware exists with that id
***************************************************************************************************/
bool_t islSdkFirmwareGetInfo(uint32_t id, firmwareInfo_t* info);

/***************************************************************************************************
* Gets the firmwareInfo_t information for loaded firmware.
* Example usage.
* @code
* bool_t valid;
* firmwareInfo_t info;
* uint32_t idx = 0;
*
* do
* {
*    valid = islSdkFirmwareGetInfo(&info, idx, PID_ANY);
*    idx++;
* } while (valid);
*
* @endcode
* @param info A pointer to a firmwareInfo_t struct which will be populated
* @param index An iterator to index the list of firmware objects, start at zero and increment.
* @param filterPid Only look for firmware that matches this PID  
* @return
*		 - TRUE if the \p info struct was populated
*		 - FALSE no firmware object item exists at the passed \p index matching the \p filterPid
***************************************************************************************************/
bool_t islSdkFirmwareListGetInfo(firmwareInfo_t* info, uint32_t index, devicePid_t filterPid);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
