#ifndef ISLSDKLOGING_H_
#define ISLSDKLOGING_H_
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* @file This module contains definitions and functions for logging data
***************************************************************************************************/

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

/// When a logging event occurs a callback function with the signature ::sdkEventHandler_t is called. 
/// To set this callback see islSdkLogSetEvent(). 
/// The {@link sdkEventHandler_t id} parameter is set to the SDK assigned id of the log and the
/// {@link sdkEventHandler_t eventId} parameter is set to one of these and this essentially defines how to cast the void* {@link sdkEventHandler_t data} parameter in the callback
typedef enum
{
    EVENT_LOG_ERROR,                        ///< An error occurred. Cast void* to sdkError_t *. { @link sdkError_t errorCode } will be one of ::logErrorCodes_t
    EVENT_LOG_CREATED,                      ///< Log object created. Cast void* to str_t *
    EVENT_LOG_OPENED,                       ///< A log file has been opened. Cast void* to logInfo_t *
    EVENT_LOG_CLOSED,                       ///< A log file has been closed. void* is NULLPTR
    EVENT_LOG_TRACK_ADDED,                  ///< A track has been added to the logging object. Cast void* to uint32_t * to get the track id
    EVENT_LOG_NEW_FILE,                     ///< The log object has created a new log file. Cast void* to str_t* to get full file name
    EVENT_LOG_FILE_SIZE,                    ///< The log file size has reached or exceeded the set level as set by islSdkLogStartNewFile(). void* is NULLPTR
    EVENT_LOG_META_RECORD,                  ///< Log record meta data. Cast void* to logDataRecord_t *
    EVENT_LOG_DATA_RECORD,                  ///< Log record data. Cast void* to logDataRecord_t *
    EVENT_LOG_INFO,                         ///< Log information. Cast void* to logInfo_t *
} logEvent_t;

/// Error codes see ::EVENT_LOG_ERROR.
typedef enum
{
    ERROR_LOG_ID_INVALID,                   ///< No log object exists with this id
    ERROR_LOG_FILE_DAMAGED,                 ///< Log file wasn't closed and is missing index data
    ERROR_LOG_FILE_OPEN,                    ///< File would not open. Check file exists
    ERROR_LOG_FILE_DAMAGED_FATAL,           ///< Log file is corrupt
    ERROR_LOG_NEW_FILE_FAILED,              ///< Failed to create file
} logErrorCodes_t;

/// Details for a track within a log.
typedef struct
{
    uint32_t trackId;                       ///< Id of the track
    uint32_t appId;                         ///< User defined id. Typically a number to represent instance type of the logging app
    uint32_t pnSn;                          ///< User defined. Typically used to hold the part number (concatenation of (uint16_t){ @link deviceInfo_t pn } with (uint16_t){ @link deviceInfo_t sn })
    uint16_t pid;                           ///< User defined. Typically used to hold the ::devicePid_t
    uint32_t startRecordIndex;              ///< First record number in this track, referenced to the start of the log file 
	uint32_t recordCount;                   ///< Number of records in this track
	uint32_t startTimeMs;                   ///< Number of milliseconds since the time stamp { @link logInfo_t startTime }
	uint32_t durationMs;                    ///< Length of the track in milliseconds
} logTrackInfo_t;

typedef struct
{
	uint32_t id;                            ///< Id of the log object
    uint32_t totalRecordCount;              ///< Total number of records in the log file. Sum of { @link logTrackInfo_t recordCount } for each ::logTrackInfo_t
    uint64_t startTime;                     ///< Log file creation time as milliseconds since the Unix epoch - 00:00:00 UTC on 1 January 1970
    uint32_t totalDurationMs;               ///< Length of the log file in milliseconds
	uint32_t trackCount;                    ///< Number of elements pointed to by the logTrackInfo pointer
	logTrackInfo_t* logTrackInfo;           ///< Details for each track see logTrackInfo_t
} logInfo_t;

typedef struct
{
    uint32_t trackId;                       ///< Id of the track
    uint32_t appId;                         ///< User defined id. Typically a number to represent instance type of the logging app
    uint32_t pnSn;                          ///< User defined. Typically used to hold the part number (concatenation of (uint16_t){ @link deviceInfo_t pn } with (uint16_t){ @link deviceInfo_t sn })
    uint16_t pid;                           ///< User defined. Typically used to hold the ::devicePid_t
    uint32_t recordIndex;                   ///< Index of the record within the track. Starts from 0
    uint32_t timeMs;                        ///< Number of milliseconds since the time stamp { @link logInfo_t startTime } for this record
    uint8_t dataType;                       ///< Record data type as originally logged with the islSdkLogAddData() call. Use this to type cast the { @link logDataRecord_t data } pointer
    uint32_t size;                          ///< Number of bytes the void* data points to
    void* data;                             ///< record data as originally logged with the islSdkLogAddData() call
} logDataRecord_t;

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Sets the callback for logging events.
* @param callback A pointer to a function that will handle the logging events listed in ::logEvent_t
***************************************************************************************************/
void islSdkLogSetEvent(sdkEventHandler_t callback);

/***************************************************************************************************
* Create a logging object.
*
* @param fileName A pointer to a string containing the path and filename
* @note This won't create a log file until islSdkLogStartNewFile() is called
***************************************************************************************************/
void islSdkLogCreate(const str_t* fileName);

/***************************************************************************************************
* Open a log file.
*
* @param fileName A pointer to a string containing the path and filename
* @note The file remains open untill islSdkLogClose() is called
***************************************************************************************************/
void islSdkLogOpen(const str_t* fileName);

/***************************************************************************************************
* Close a log file.
*
* @param logId Id of the log object
***************************************************************************************************/
void islSdkLogClose(uint32_t logId);

/***************************************************************************************************
* Add a track to the log object.
* Tracks are required to log data to and be added before or after a islSdkLogStartNewFile() call
*
* @param logId Id of the log object
* @param trackId User assigned id for the track. This must be unique to the log instance
* @param appId A user defined value. Typically a number to represent instance type of the logging app
* @param partNumber A user defined value. Typically used to hold the part number (concatenation of (uint16_t){ @link deviceInfo_t pn } with (uint16_t){ @link deviceInfo_t sn })
* @param pid A user defined value. Typically used to hold the ::devicePid_t
***************************************************************************************************/
void islSdkLogAddTrack(uint32_t logId, uint32_t trackId, uint32_t appId, uint32_t partNumber, uint16_t pid);

/***************************************************************************************************
* Create a new log file.
*
* @param logId Id of the log object
* @param maxSize When the logfile reaches or exceeds this size in bytes the ::EVENT_LOG_FILE_SIZE event is
* triggered. The user can then call islSdkLogStartNewFile() to carry on logging to a new file. If this value is zero
* then no ::EVENT_LOG_FILE_SIZE event is triggered
***************************************************************************************************/
void islSdkLogStartNewFile(uint32_t logId, uint32_t maxSize);

/***************************************************************************************************
* Add data to a log file.
* This writes the passed data to a track in the current log file
*
* @param logId Id of the log object
* @param trackId Id of the track to write the log data to
* @param meta
* - TRUE when playing back the record will trigger an ::EVENT_LOG_META_RECORD event
* - FALSE when playing back the record will trigger an ::EVENT_LOG_DATA_RECORD event
* @param canSkip When true the logged records can be skipped on playback when seeking though the track
* @param dataType A user defined value to define what \p data is, so it can be interpreted on playback 
* @param data Pointer to the data to write to file
* @param size Size in bytes of the data
***************************************************************************************************/
void islSdkLogAddData(uint32_t logId, uint32_t trackId, bool_t meta, bool_t canSkip, uint8_t dataType, const void* data, uint32_t size);

/***************************************************************************************************
* Play a log object.
*
* @param logId Id of the log object
* @param speed The speed to play the log file at. 1.0 is real time. 0 stops playback and negative numbers play backwards
***************************************************************************************************/
void islSdkLogPlay(uint32_t logId, real_t speed);

/***************************************************************************************************
* Seek through a log file.
*
* @param logId Id of the log object
* @param index The record index to jump to. When jumps are very large only the final 50 records
* and records where canSkip is FASLE are outputted
***************************************************************************************************/
void islSdkLogSeek(uint32_t logId, uint32_t index);

/***************************************************************************************************
* Get the log file details.
* This triggers an ::EVENT_LOG_INFO event
*
* @param logId Id of the log object
***************************************************************************************************/
void islSdkLogGetInfo(uint32_t logId);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
