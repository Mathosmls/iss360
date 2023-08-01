#ifndef TIMEUTILS_H_
#define TIMEUTILS_H_
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------ Includes ----------------------------------------------

#include "sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dayOfWeek;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t millisecond;
} timeVals_t;

/// Platform time function pointers
typedef struct
{
    /***************************************************************************************************
    * Function pointer to get the time
    * @return Time in milliseconds since the Unix epoch - 00:00:00 UTC on 1 January 1970
    ***************************************************************************************************/
    uint64_t(*getTime)(void);

    /***************************************************************************************************
    * Function pointer convert the millisecond Unix epoch time stamp into a timeVals_t struct
    * @param time Time in milliseconds since the Unix epoch
    * @param timeVals Pointer to timeVals_t struct where the values will be populated
    ***************************************************************************************************/
    void(*convert)(uint64_t time, timeVals_t* timeVals);

    /***************************************************************************************************
    * Function pointer convert to return the frequency of the tick counter
    * @return the frequency of the tick counter
    ***************************************************************************************************/
    uint64_t(*getTickFrequency)(void);

    /***************************************************************************************************
    * Function pointer to get the current tick count
    * @return the tick count
    ***************************************************************************************************/
    uint64_t(*getTicks)(void);
} timeFunctions_t;

extern timeFunctions_t timeFunctions;

//---------------------------------- Public Function Prototypes -----------------------------------

void timeInitialise(void);

/***************************************************************************************************
* Get the time that the SDK was started in milliseconds
* @return The number of milliseconds between the Unix epoch - 00:00:00 UTC on 1 January 1970 and the starting of the SDK
* @note Add this to the timeMs parameter of events to get the UTC time stamp of the event
* @sa timeFunctions_t::convert
***************************************************************************************************/
uint64_t timeGetAppStartTime(void);

/***************************************************************************************************
* Get the number of milliseconds since the SDK was started
* @return The number of milliseconds
***************************************************************************************************/
uint64_t timeGetMsCounter(void);

/***************************************************************************************************
* Get the number of seconds between 2 counter values
* @param ticks1 A count value of a timer
* @param ticks2 A count value of a timer
* @return The number of seconds between 2 tick values
* @warning The difference between \p ticks1 and \p ticks2 must not exceed half the counter rollover value
***************************************************************************************************/
real_t timeSecondsBetween(uint64_t ticks1, uint64_t ticks2);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif