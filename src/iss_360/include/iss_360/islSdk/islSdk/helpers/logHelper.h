#ifndef LOGHELPER_H_
#define LOGHELPER_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "islSdkLogging.h"
#include "islSdkDevice.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Copy the log file record into an SDK defined struct.
* This function takes the raw log data from an ::EVENT_LOG_DATA_RECORD or ::EVENT_LOG_META_RECORD and
* copies the packed / serialised data into a struct.
* @param logDataRecord A pointer to the {@link logDataRecord_t logDataRecord} struct
* @return 
* - TRUE Record was processed and logDataRecord was updated
* - FALSE logDataRecord was not altered as the {@link logDataRecord t dataType} isn't supported for the pid
***************************************************************************************************/
bool_t logHelperProcessRecord(logDataRecord_t* logDataRecord);

/***************************************************************************************************
* Log the passed event data to file.
* This function essentially packs / serialises certain structures before passing to the islSdkLogAddData() function.
* This is important as structure padding can change between compliers and architectures
* @param logId The Id of the log object to log to
* @param trackId The track to wirte to
* @param pid The product id
* @param eventId The event id e.g. one of ::deviceEvent_t, ::iss360Event_t, etc.
* @param data A pointer to the data from one of the events e.g. ::deviceEvent_t, ::iss360Event_t, etc.
* - TRUE data was logged
* - FALSE data did not log
***************************************************************************************************/
bool_t logHelperLogData(uint32_t logId, uint32_t trackId, devicePid_t pid, uint32_t eventId, const void* data);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
