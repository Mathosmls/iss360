#ifndef PLATFORMUART_H_
#define PLATFORMUART_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "platform/uart.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types -------------------------------------------

//---------------------------------- Public Function Prototypes ------------------------------------

uint32_t platformUartGetPorts(utf8_t* buf, uint32_t bufSize);
void* platformUartOpen(const utf8_t* portStr);
void platformUartClose(void* handle);
bool_t platformUartConfig(void* handle, uint32_t baudrate, uint8_t dataBits, uartParity_t parity, uartStopBits_t stopBits);
bool_t platformUartWrite(void* handle, const uint8_t* data, uint32_t* size);
bool_t platformUartRead(void* handle, uint8_t* buf, uint32_t* size);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
