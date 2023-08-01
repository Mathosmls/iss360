
#ifndef SDKTYPES_H_
#define SDKTYPES_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include <stdint.h>
#include <stdbool.h>

//--------------------------------------- Public Constants -----------------------------------------

#define NULLPTR    ((void*)0)
#define TRUE    1
#define FALSE   0
#define MATH_USE_DOUBLE 1

//----------------------------------------- Public Types -------------------------------------------

typedef bool bool_t;
typedef float float_t;
typedef double double_t;
typedef float float32_t;
typedef double double64_t;
typedef uintptr_t ptr_t;
typedef char str_t;
typedef str_t utf8_t;

#if MATH_USE_DOUBLE != 0
typedef double real_t;
#else
typedef float real_t;
#endif

/// Error struct for error events
typedef struct
{
	uint32_t errorCode;		///< error code
	str_t* msg;			///< error message string
} sdkError_t;

typedef struct
{
	real_t x;
	real_t y;
} point_t;

#define countof(arr) (sizeof(arr) / sizeof((arr)[0]))

/***************************************************************************************************
* Function signature of the SDK events
* @param timeMs Time stamp in milliseconds since islSdkInitialise() was called
* @param eventId The id of the event. This defines how to cast the void* \p data parameter
* @param id The id of the object that raised the event as assigned by the SDK
* @param data The event's data
***************************************************************************************************/
typedef void(*sdkEventHandler_t)(uint64_t timeMs, uint32_t eventId, uint32_t id, const void* data);

#define TXT(str) str
#define STR(str) (const str_t*)str
//#define TXT(str) L##str
//#define STR(str) (const utf16_t*)L##str

//----------------------------------- Public Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
