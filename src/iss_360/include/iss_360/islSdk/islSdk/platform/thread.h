#ifndef THREAD_H_
#define THREAD_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

typedef void thread_t;

/// Platform thread function pointers
typedef struct
{
    /***************************************************************************************************
    * Function pointer to create a thread
    * @return Pointer to the thread object
    ***************************************************************************************************/
    thread_t*(*create)(void*(*func)(void*));
    
    /***************************************************************************************************
    * Function pointer to sleep the thread
    * @param ms Number of milliseconds to sleep
    ***************************************************************************************************/
    void(*sleepMs)(uint32_t ms);

    /***************************************************************************************************
    * Function pointer to terminate and join the thread to the calling thread
    * @param Pointer to the thread object
    ***************************************************************************************************/
    void(*join)(thread_t* thread);
} threadFunctions_t;

extern threadFunctions_t threadFunctions;

//---------------------------------- Public Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif