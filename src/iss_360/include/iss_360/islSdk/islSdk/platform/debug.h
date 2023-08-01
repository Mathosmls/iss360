#ifndef DEBUG_H_
#define DEBUG_H_
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------ Includes ----------------------------------------------

#include "sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

#define PRINT_ON
//#define DEBUG_ON
#define INFO_ON
#define WARNING_ON
#define ERROR_ON

#ifdef PRINT_ON
#define printLog(fmt, ...) islSdkDebugPrintMsg(STR(fmt), ##__VA_ARGS__)
#else
#define printLog(fmt, ...)
#endif

#ifdef DEBUG_ON
#define debugLog(fmt, ...) islSdkDebugPrintMsg((const str_t*)(TXT("DEBUG: ") fmt TXT("\n")), ##__VA_ARGS__)
#define debugAssert(expr) do {if (!(expr)) { errorLog(TXT("Invalid variable in funciton: %s line: %d file: %s"), __func__,  __LINE__ , __FILE__ ); while (1);}} while(0)
#else
#define debugLog(fmt, ...)
#define debugAssert(expr)
#endif

#ifdef INFO_ON
#define infoLog(fmt, ...) islSdkDebugPrintMsg((const str_t*)(TXT("INFO: ") fmt TXT("\n")), ##__VA_ARGS__)
#else
#define infoLog(fmt, ...)
#endif

#ifdef WARNING_ON
#define warningLog(fmt, ...) islSdkDebugPrintMsg((const str_t*)(TXT("WARNING: ") fmt TXT("\n")), ##__VA_ARGS__)
#else
#define warningLog(fmt, ...)
#endif

#ifdef ERROR_ON
#define errorLog(fmt, ...) islSdkDebugPrintMsg((const str_t*)(TXT("ERROR: ") fmt TXT("\n")), ##__VA_ARGS__)
#else
#define errorLog(fmt, ...)
#endif


//----------------------------------------- Public Types ------------------------------------------

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Formats a message and calls the print funciton pointer (if set)
* @param msg A pointer the formating string
* @param ... A variable list of arguments
***************************************************************************************************/
void islSdkDebugPrintMsg(const str_t* msg, ...);

/***************************************************************************************************
* Sets the print funciton pointer
* @param printFunc A pointer to a function that outputs messages
***************************************************************************************************/
void islSdkDebugSetPrintFunc(void(*printFunc)(const str_t* msg, uint32_t length));

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
