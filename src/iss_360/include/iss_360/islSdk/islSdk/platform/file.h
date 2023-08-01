
#ifndef FILE_H_
#define FILE_H_
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------ Includes ----------------------------------------------

#include "sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

typedef void file_t;

/// Platform file function pointers
typedef struct
{
    /***************************************************************************************************
    * Function pointer to create or overwrite a file.
    * @param fileName The path and file name as a string
    * @return Pointer to the file object. NULLPTR if file creation failed
    ***************************************************************************************************/
    file_t*(*create)(const str_t* fileName);

    /***************************************************************************************************
    * Function pointer to open a file.
    * @param fileName The path and file name as a string
    * @param readOnly If true opens in read only mode
    * @return Pointer to the file object. NULLPTR if file creation failed
    ***************************************************************************************************/
    file_t*(*open)(const str_t* fileName, bool_t readOnly);

    /***************************************************************************************************
    * Function pointer to closes a file.
    * @param file Pointer to the file object
    * @return TRUE if succeeded
    ***************************************************************************************************/
    bool_t(*close)(file_t* file);

    /***************************************************************************************************
    * Function pointer to seek though a file.
    * @param file Pointer to the file object
    * @param position The byte offset
    * @param relative If true \p position is referenced from the current file position
    * @return True if succeeded
    ***************************************************************************************************/
    bool_t(*seek)(file_t* file, int32_t position, bool_t relative);

    /***************************************************************************************************
    * Function pointer to read a file.
    * @param file Pointer to the file object
    * @param buf Pointer to a buffer where the file is read into
    * @param length Number of bytes to read
    * @return Number of bytes read
    ***************************************************************************************************/
    uint32_t(*read)(file_t* file, void* buf, uint32_t length);

    /***************************************************************************************************
    * Function pointer to read a line from a file.
    * @param file Pointer to the file object
    * @param buf Pointer to a buffer where the file is read into
    * @param maxLength Size of the buffer \p buf
    * @return Number of bytes read
    ***************************************************************************************************/
    uint32_t(*readLine)(file_t* file, str_t* buf, uint32_t maxLength);

    /***************************************************************************************************
    * Function pointer to write to a file.
    * @param file Pointer to the file object
    * @param data Pointer to a buffer where data the data to write is
    * @param length Number of bytes to write
    * @return True if succeeded
    ***************************************************************************************************/
    bool_t(*write)(file_t* file, const void* data, uint32_t length);
} fileFunctions_t;

extern fileFunctions_t fileFunctions;

//---------------------------------- Public Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif