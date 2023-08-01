//------------------------------------------ Includes ----------------------------------------------

#include "platformTime.h"
#include "windows.h"

//--------------------------------------- Public Constants -----------------------------------------

#define TICKS_PER_MILLISECOND (uint64_t)10000
#define MILLISECONDS_TO_UNIX_EPOCH (uint64_t)11644473600000

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

//---------------------------------- Private Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
uint64_t platformTimeGet(void)
{
	FILETIME fileTime;
	uint64_t time64;

	GetSystemTimeAsFileTime(&fileTime);												// Number of 100-nanosecond intervals since January 1, 1601 (UTC)
	time64 = ((uint64_t)fileTime.dwHighDateTime << 32) | fileTime.dwLowDateTime;
	time64 = (time64 / TICKS_PER_MILLISECOND) - MILLISECONDS_TO_UNIX_EPOCH;			// Convert to number of milliseconds since January 1, 1970 (UTC)

	return time64;
}
//--------------------------------------------------------------------------------------------------
void platformTimeConvert(uint64_t timeMs, timeVals_t* timeVals)
{
	FILETIME fileTime;
	SYSTEMTIME systemTime;

	timeMs = (timeMs + MILLISECONDS_TO_UNIX_EPOCH) * TICKS_PER_MILLISECOND;			// Conver to number of 100-nanosecond intervals since January 1, 1601 (UTC)

	fileTime.dwHighDateTime = timeMs >> 32;
	fileTime.dwLowDateTime = (uint32_t)timeMs;

	FileTimeToSystemTime(&fileTime, &systemTime);
	
	timeVals->year = (uint16_t)systemTime.wYear;
	timeVals->month = (uint8_t)systemTime.wMonth;
	timeVals->day = (uint8_t)systemTime.wDay;
	timeVals->dayOfWeek = (uint8_t)systemTime.wDayOfWeek;
	timeVals->hour = (uint8_t)systemTime.wHour;
	timeVals->minute = (uint8_t)systemTime.wMinute;
	timeVals->second = (uint8_t)systemTime.wSecond;
	timeVals->millisecond = (uint16_t)systemTime.wMilliseconds;
}
//--------------------------------------------------------------------------------------------------
uint64_t platformTimeGetTickFrequency(void)
{
	LARGE_INTEGER clkFreqency;

	QueryPerformanceFrequency(&clkFreqency);

	return (uint64_t)clkFreqency.QuadPart;
}
//--------------------------------------------------------------------------------------------------
uint64_t platformTimeGetTicks(void)
{
	LARGE_INTEGER ticks;

	QueryPerformanceCounter(&ticks);

	return (uint64_t)ticks.QuadPart;
}
//--------------------------------------------------------------------------------------------------
