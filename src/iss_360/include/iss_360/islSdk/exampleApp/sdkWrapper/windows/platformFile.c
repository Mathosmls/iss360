//------------------------------------------ Includes ----------------------------------------------

#include "platformFile.h"
#include "windows.h"
#include <stdlib.h>

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

static uint16_t utf16Buf[256];

//---------------------------------- Private Function Prototypes -----------------------------------

static uint16_t* utf8ToUtf16(const utf8_t* utf8);
static void cleanUp(uint16_t* wStr);

//--------------------------------------------------------------------------------------------------
void* platformFileCreate(const utf8_t* fileName)
{
	void* handle;
	uint16_t* wStrBuf;
	uint16_t* wStr;
	uint16_t* fn;

	handle = NULLPTR;
	wStrBuf = utf8ToUtf16(fileName);
	
	if (wStrBuf != NULLPTR)
	{
		wStr = wStrBuf;
		fn = NULLPTR;
		while (*wStr)
		{
			if (*wStr == L'/' || *wStr == L'\\')
			{
				*wStr = L'\\';
				fn = wStr;
			}
			wStr++;
		}

		if (fn != NULL)
		{
			*fn = 0;
			CreateDirectoryW(wStrBuf, NULLPTR);
			*fn = L'\\';

			handle = CreateFileW(wStrBuf, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

			if (handle == INVALID_HANDLE_VALUE)
			{
				handle = NULLPTR;
			}
		}	
	}

	cleanUp(wStrBuf);

	return handle;
}
//--------------------------------------------------------------------------------------------------
void* platformFileOpen(const utf8_t* fileName, bool_t readOnly)
{
    void* handle;
	uint32_t dwDesiredAccess = GENERIC_READ;
	uint32_t dwShareMode = 0;
	uint32_t dwCreationDisposition = OPEN_EXISTING;
	uint16_t* wStr;

	handle = NULLPTR;
	wStr = utf8ToUtf16(fileName);

	if (wStr != NULLPTR)
	{
		if (readOnly)
		{
			dwShareMode = FILE_SHARE_READ;
		}
		else
		{
			dwDesiredAccess |= GENERIC_WRITE;
		}

		handle = CreateFileW(wStr, dwDesiredAccess, dwShareMode, 0, dwCreationDisposition, 0, NULLPTR);

		if (handle == INVALID_HANDLE_VALUE)
		{
			handle = NULLPTR;
		}
	}

	cleanUp(wStr);
	return handle;
}
//--------------------------------------------------------------------------------------------------
bool_t platformFileClose(void* handle)
{
	if (handle != NULLPTR)
	{
		return CloseHandle(handle) != 0;
	}

    return FALSE;
}
//--------------------------------------------------------------------------------------------------
bool_t platformFileSeek(void* handle, int32_t position, bool_t relative)
{
	uint32_t dwMoveMethod = 0;

	dwMoveMethod = relative == TRUE;

	if (handle != NULLPTR)
	{
		return SetFilePointer(handle, position, NULL, dwMoveMethod) != INVALID_SET_FILE_POINTER;
	}

    return FALSE;
}
//--------------------------------------------------------------------------------------------------
uint32_t platformFileRead(void* handle, void* buf, uint32_t length)
{
	uint32_t bytesRead = 0;

	if (handle != NULLPTR)
	{
		ReadFile(handle, buf, length, &bytesRead, NULLPTR);
	}

    return bytesRead;
}
//--------------------------------------------------------------------------------------------------
uint32_t platformFileReadLineUtf8(void* handle, utf8_t* buf, uint32_t maxLength)
{
	uint32_t bytesRead;
	uint32_t i = 0;
	uint32_t lineLength = 0;
	utf8_t* line = buf;

	if (handle != NULLPTR)
	{
		bytesRead = platformFileRead(handle, line, maxLength-1);

		for (i = 0; i < bytesRead; i++)
		{
			if (line[i] == '\n')
			{
				i++;
				break;
			}
		}
		line[i] = 0;
		platformFileSeek(handle, (int32_t)(bytesRead - i) * -1, 1);
	}

	return i;
}
//--------------------------------------------------------------------------------------------------
bool_t platformFileWrite(void* handle, const void* data, uint32_t length)
{
	uint32_t bytesWritten = 0;

	if (handle != NULLPTR)
	{
		WriteFile(handle, data, length, &bytesWritten, NULLPTR);
	}

	return length == bytesWritten;
}
//--------------------------------------------------------------------------------------------------
static uint16_t* utf8ToUtf16(const utf8_t* utf8)
{
	uint32_t wStrLen;
	uint16_t* wStrMem;
	uint16_t* wStr;

	wStrMem = NULLPTR;
	wStrLen = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, utf8, -1, NULLPTR, 0);

	if (wStrLen >= (sizeof(utf16Buf) / sizeof(uint16_t)))
	{
		wStrMem = malloc(wStrLen * sizeof(uint16_t));
		wStr = wStrMem;
	}
	else
	{
		wStr = &utf16Buf[0];
	}

	if (MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, utf8, -1, wStr, wStrLen) == 0)
	{
		wStr = NULLPTR;
	}

	return wStr;
}
//--------------------------------------------------------------------------------------------------
static void cleanUp(uint16_t* wStr)
{
	if (wStr != NULLPTR && wStr != &utf16Buf[0])
	{
		free(wStr);
	}
}
//--------------------------------------------------------------------------------------------------
