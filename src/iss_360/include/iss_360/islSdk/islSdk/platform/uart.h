#ifndef UART_H_
#define UART_H_
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------ Includes ----------------------------------------------

#include "sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

typedef void uart_t;

typedef enum
{
	UART_NO_PARITY = 0,
	UART_ODD_PARITY = 1,
	UART_EVEN_PARITY = 2,
	UART_MARK_PARITY = 3,
	UART_SPACE_PARITY = 4,
	UART_UNKNOWN_PARITY,
} uartParity_t;

typedef enum
{
	UART_1_STOPBIT = 0,
	UART_1HALF_STOPBIT = 1,
	UART_2_STOPBIT = 2,
	UART_UNKNOWN_STOPBIT,
} uartStopBits_t;

/// Platform time function pointers
typedef struct
{
	/***************************************************************************************************
	* Function pointer to get the serial port names.
	* @param buf Pointer to a buffer where the names should be written. Each name should be seperated by a NULL
	* and terminated by 2 sequential NULL's. e.g "COM1\0COM2\0COM3\0\0"
	* @param size The size of the buffer \p buf
	* @return The number of ports
	***************************************************************************************************/
	uint32_t(*getPortNames)(str_t* buf, uint32_t size);

	/***************************************************************************************************
	* Function pointer to open a serial port.
	* @param name Pointer to a name as returned in the list by getPortNames()
	* @return Pointer to the serial port object
	***************************************************************************************************/
    uart_t* (*open)(const str_t* name);

	/***************************************************************************************************
	* Function pointer to close a serial port.
	* @param inst Pointer to the serial port object
	***************************************************************************************************/
    void (*close)(uart_t* inst);

	/***************************************************************************************************
	* Function pointer to configure a serial port.
	* @param inst Pointer to the serial port object
	* @param baudrate The baudrate
	* @param dataBits Number of data bits 5,6,7,8,9
	* @param parity Parity option
	* @param stopBits Stop bits option
	* @return True if succeeded
	***************************************************************************************************/
    bool_t (*config)(uart_t* inst, uint32_t baudrate, uint8_t dataBits, uartParity_t parity, uartStopBits_t stopBits);

	/***************************************************************************************************
	* Function pointer to write to a serial port.
	* @param inst Pointer to the serial port object
	* @param data Pointer to the data to write
	* @param size Number of bytes to write
	***************************************************************************************************/
    bool_t (*write)(uart_t* inst, const uint8_t* data, uint32_t* size);

	/***************************************************************************************************
	* Function pointer to read a serial port.
	* This function should not block or wait for bytes. Set \p *size = 0 and return if no data is available
	* @param inst Pointer to the serial port object
	* @param buf Pointer to a buffer to read serial port data into
	* @param size Pointer to the size of the buffer \p buf in bytes. This is updated with the number of bytes read
	* @return True if succeeded
	***************************************************************************************************/
	bool_t (*read)(uart_t* inst, uint8_t* buf, uint32_t* size);
} uartFunctions_t;

extern uartFunctions_t uartFunctions;

//---------------------------------- Public Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
