//------------------------------------------ Includes ----------------------------------------------

#include "platformTime.h"
#include <sys/time.h>
#include <time.h>

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

//---------------------------------- Private Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
uint64_t platformTimeGet(void)
{
    uint64_t timeNow;
    struct timeval tp;

    gettimeofday(&tp, NULLPTR);
    timeNow = (tp.tv_sec * 1000) + (tp.tv_usec / 1000);

	return timeNow;
}
//--------------------------------------------------------------------------------------------------
void platformTimeConvert(uint64_t timeMs, timeVals_t* timeVals)
{
	struct tm* utcTime;
    uint64_t timeS;

	timeS = timeMs / 1000;
	utcTime = gmtime((time_t*)&timeS);

    timeVals->year = (uint16_t)utcTime->tm_year + 1900;
    timeVals->month = (uint8_t)utcTime->tm_mon;
    timeVals->day = (uint8_t)utcTime->tm_mday;
    timeVals->dayOfWeek = (uint8_t)utcTime->tm_wday;
    timeVals->hour = (uint8_t)utcTime->tm_hour;
    timeVals->minute = (uint8_t)utcTime->tm_min;
    timeVals->second = (uint8_t)utcTime->tm_sec;
    timeVals->millisecond = timeMs - (timeS * 1000);
}
//--------------------------------------------------------------------------------------------------
uint64_t platformTimeGetTickFrequency(void)
{
	uint64_t clkFreqency;

	clkFreqency = CLOCKS_PER_SEC;

	return clkFreqency;
}
//--------------------------------------------------------------------------------------------------
uint64_t platformTimeGetTicks(void)
{
	clock_t ticks;

	ticks = clock();

	//struct timespec ts;
	//clock_gettime(CLOCK_MONOTONIC, &ts);
    //return (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);

	return (uint64_t)ticks;
}
//--------------------------------------------------------------------------------------------------
