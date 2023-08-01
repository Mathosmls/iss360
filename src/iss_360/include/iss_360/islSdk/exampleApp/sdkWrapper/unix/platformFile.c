//------------------------------------------ Includes ----------------------------------------------

#include "platformFile.h"
#include <stdio.h>

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Local Types --------------------------------------------

//------------------------------------------- Globals ----------------------------------------------

//---------------------------------- Private Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
void* platformFileCreate(const utf8_t* fileName)
{
	FILE* file;

    file = fopen((char*)fileName, TXT("wb+"));

	return file;
}
//--------------------------------------------------------------------------------------------------
void* platformFileOpen(const utf8_t* fileName, bool_t readOnly)
{
    FILE* file;

	if (readOnly)
	{
		file = fopen((char*)fileName, TXT("rb"));
	}
	else
	{
		file = fopen((char*)fileName, TXT("rb+"));
	}

	return file;
}
//--------------------------------------------------------------------------------------------------
bool_t platformFileClose(void* handle)
{
	if (handle != NULLPTR)
	{
		return fclose((FILE*)handle) == 0;
	}

	return FALSE;
}
//--------------------------------------------------------------------------------------------------
bool_t platformFileSeek(void* handle, int32_t position, bool_t relative)
{
	int32_t origin;

	if (handle != NULLPTR)
	{
		if (relative)
		{
			origin = SEEK_CUR;
		}
		else
		{
			origin = SEEK_SET;
		}

		return fseek((FILE*)handle, position, origin) == 0;
	}

	return FALSE;
}
//--------------------------------------------------------------------------------------------------
uint32_t platformFileRead(void* handle, void* buf, uint32_t length)
{
	uint32_t bytesRead = 0;

	if (handle != NULLPTR)
	{
		bytesRead = fread(buf, 1, length, (FILE*)handle);
	}

    return bytesRead;
}
//--------------------------------------------------------------------------------------------------
uint32_t platformFileReadLineUtf8(void* handle, utf8_t* buf, uint32_t maxLength)
{
	uint32_t bytesRead;
	uint32_t i = 0;
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
		bytesWritten = fwrite(data, 1, length, (FILE*)handle);
	}

	return length == bytesWritten;
}
//--------------------------------------------------------------------------------------------------
