//------------------------------------------ Includes ----------------------------------------------

#include "platformUart.h"
#include "windows.h"
#include <aclapi.h>
#include <winerror.h>
#include "platform/debug.h"

#pragma comment(lib, "advapi32.lib")

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

//---------------------------------- Private Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
uint32_t platformUartGetPorts(utf8_t* buf, uint32_t bufSize)
{
	HKEY regKey;
	bool_t moreData = TRUE;
	uint32_t idx = 0;
    uint16_t name[255];
	uint32_t nameSize;
	uint32_t portNameSize;
	uint16_t portName[255];
	uint32_t portCount = 0;

    if (bufSize != 0)
    {
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &regKey) == ERROR_SUCCESS)
        {
            bufSize--;
            while (moreData)
            {
                nameSize = sizeof(name) / sizeof(name[0]);
                portNameSize = sizeof(portName);

                moreData = RegEnumValueW(regKey, idx, &name[0], &nameSize, 0, 0, (LPBYTE)&portName[0], &portNameSize) == ERROR_SUCCESS;
                idx++;

                if ((portNameSize != sizeof(portName)) && (portNameSize < bufSize))
                {
                    portNameSize = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, &portName[0], -1, buf, bufSize, NULL, NULL);
                    bufSize -= portNameSize;
                    buf += portNameSize;
                    portCount++;
                }
            }
            RegCloseKey(regKey);
        }
        *buf = 0;
    }
    return portCount;
}
//--------------------------------------------------------------------------------------------------
void* platformUartOpen(const utf8_t* portStr)
{
    COMMTIMEOUTS commTimeOut;
    DCB dcbCommPort;
    void* handle;
    uint16_t str[32] = {L"\\\\.\\"};


    if (!MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, portStr, -1, &str[4], (sizeof(str) / sizeof(uint16_t)) - 4))
    {
        return NULLPTR;
    }

    handle = CreateFileW(&str[0], GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

    if (handle != INVALID_HANDLE_VALUE)
    {
        commTimeOut.ReadIntervalTimeout = 0xffffffff;
        commTimeOut.ReadTotalTimeoutConstant = 0;
        commTimeOut.ReadTotalTimeoutMultiplier = 0;
        commTimeOut.WriteTotalTimeoutMultiplier = 0;
        commTimeOut.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(handle, &commTimeOut);

        dcbCommPort.DCBlength = sizeof(DCB);
        GetCommState(handle, &dcbCommPort);
        dcbCommPort.BaudRate = 9600;
        dcbCommPort.fBinary = 1;
        dcbCommPort.fParity = 0;
        dcbCommPort.fOutxCtsFlow = 0;
        dcbCommPort.fOutxDsrFlow = 0;
        dcbCommPort.fDtrControl = DTR_CONTROL_DISABLE;
        dcbCommPort.fDsrSensitivity = 0;
        dcbCommPort.fTXContinueOnXoff = 0;
        dcbCommPort.fOutX = 0;
        dcbCommPort.fInX = 0;
        dcbCommPort.fErrorChar = 0;
        dcbCommPort.fNull = 0;
        dcbCommPort.fRtsControl = RTS_CONTROL_DISABLE;
        dcbCommPort.fAbortOnError = 0;
        dcbCommPort.ByteSize = 8;
        dcbCommPort.Parity = NOPARITY;
        dcbCommPort.StopBits = ONESTOPBIT;

        SetCommState(handle, &dcbCommPort);
    }
    else
    {
        handle = NULLPTR;
    }

    return handle;
}
//--------------------------------------------------------------------------------------------------
void platformUartClose(void* handle)
{
    debugAssert(handle != NULLPTR);
    CloseHandle(handle);
}
//--------------------------------------------------------------------------------------------------
bool_t platformUartConfig(void* handle, uint32_t baudrate, uint8_t dataBits, uartParity_t parity, uartStopBits_t stopBits)
{
    DCB dcbCommPort;
    bool_t ok;

    debugAssert(handle != NULLPTR);

    FlushFileBuffers(handle);

    dcbCommPort.DCBlength = sizeof(DCB);
    GetCommState(handle, &dcbCommPort);
    dcbCommPort.BaudRate = baudrate;
    dcbCommPort.fParity = parity != 0;
    dcbCommPort.Parity = parity;
    dcbCommPort.ByteSize = dataBits;
    dcbCommPort.StopBits = stopBits;

    ok = SetCommState(handle, &dcbCommPort) != 0;
    PurgeComm(handle, PURGE_RXCLEAR);

    return ok;
}
//--------------------------------------------------------------------------------------------------
bool_t platformUartWrite(void* handle, const uint8_t* data, uint32_t* size)
{
    uint32_t bytesWritten;
    uint32_t bytesRemaining;
    uint32_t attempts;

    debugAssert(handle != NULLPTR);

    attempts = 4;
    bytesWritten = 0;
    bytesRemaining = *size;

    while (attempts && bytesRemaining)
	{
        if (!WriteFile(handle, &data[bytesWritten], bytesRemaining, &bytesWritten, 0))
        {
            *size -= bytesRemaining;
            return FALSE;
        }
        bytesRemaining -= bytesWritten;
		attempts--;
	}

    *size -= bytesRemaining;

    return TRUE;
}
//--------------------------------------------------------------------------------------------------
bool_t platformUartRead(void* handle, uint8_t* buf, uint32_t* size)
{
    uint32_t bufSize = *size;

    debugAssert(handle != NULLPTR);
	*size = 0;
    return ReadFile(handle, buf, bufSize, size, 0);
}
//--------------------------------------------------------------------------------------------------
