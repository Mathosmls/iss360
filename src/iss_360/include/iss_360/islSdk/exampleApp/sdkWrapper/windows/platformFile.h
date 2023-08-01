#ifndef PLATFORMFILE_H_
#define PLATFORMFILE_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types -------------------------------------------

//---------------------------------- Public Function Prototypes ------------------------------------

void* platformFileCreate(const utf8_t* fileName);
void* platformFileOpen(const utf8_t* fileName, bool_t readOnly);
bool_t platformFileClose(void* handle);
bool_t platformFileSeek(void* handle, int32_t position, bool_t relative);
uint32_t platformFileRead(void* handle, void* buf, uint32_t length);
uint32_t platformFileReadLineUtf8(void* handle, utf8_t* buf, uint32_t maxLength);
bool_t platformFileWrite(void* handle, const void* data, uint32_t length);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
