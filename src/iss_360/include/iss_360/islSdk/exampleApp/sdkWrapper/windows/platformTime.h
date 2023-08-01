#ifndef PLATFORMTIME_H_
#define PLATFORMTIME_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "platform/timeUtils.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types -------------------------------------------

//---------------------------------- Public Function Prototypes ------------------------------------

uint64_t platformTimeGet(void);
void platformTimeConvert(uint64_t timeMs, timeVals_t* timeVals);
uint64_t platformTimeGetTickFrequency(void);
uint64_t platformTimeGetTicks(void);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
