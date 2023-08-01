#ifndef SONARIMAGE_H_
#define SONARIMAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "islSdkIss360.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

/// Type to define a sector
typedef struct
{
	uint16_t start;					///< Sector start 0 - 12800 is 0 - 360 degrees
	uint16_t size;					///< Sector size 0 - 12800 is 0 - 360 degrees
} sector_t;

/// A sector segment
typedef struct
{
	sector_t sector;
	uint32_t minRangeMm;			///< Lower range in millimeters
	uint32_t maxRangeMm;			///< Upper range in millimeters
} sonarArea_t;

/// A buffer to render into
typedef struct
{
	uint8_t* mem;					///< A pointer to the pixel buffer
	uint32_t size;					///< size of buf in bytes
	uint16_t width;					///< Width of the buffer in pixels
	uint16_t height;				///< Height of the buffer in pixels
	sonarArea_t area;				///< The sector segment of a circle that buf should hold
	bool_t useBilinerInterpolation;
	bool_t use32BitBuffer;
} sonarImageBuf_t;

/// A gradient node type
typedef struct
{
	uint32_t colour;				///< Colour value expressed as your application requires. eg ARGB or RGBA
	uint16_t position;				///< Postion of this colour in the 16 bit pallet 0x0000 to 0xffff
} gradientValue_t;

typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
} pixelArea_t;

typedef struct islSdkSonarImage_t islSdkSonarImage_t;
typedef struct islSdkpalette_t islSdkpalette_t;

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Create a palette.
*
* @return A pointer to the object
***************************************************************************************************/
islSdkpalette_t* islSdkPaletteCreate(void);

/***************************************************************************************************
* Delete a palette.
*
* @param inst A pointer to the object
***************************************************************************************************/
void islSdkPaletteDelete(islSdkpalette_t* inst);

/***************************************************************************************************
* Set the colour palette
*
* @param inst A pointer to the object
* @param gradientList A Pointer to an array of gradientValue_t
* @param count Number of elements in the array
* @param nullColour The 32 bit vlaue to use as a NULL colour
***************************************************************************************************/
void islSdkPaletteSet(islSdkpalette_t* inst, const gradientValue_t* gradientList, uint32_t count, uint32_t nullColour);

/***************************************************************************************************
* Render the palette into a buffer
*
* @param inst A pointer to the object
* @param buf A pointer to the buffer. Size must = width * height * sizeof(uint32_t)
* @param width Width in pixels
* @param height Height in pixels
* @param horizontal If true render horizontally
***************************************************************************************************/
void islSdkPaletteRender(islSdkpalette_t* inst, uint32_t* buf, uint32_t width, uint32_t height, bool_t horizontal);

/***************************************************************************************************
* Create a sonar image helper.
*
* @return A pointer to the object
***************************************************************************************************/
islSdkSonarImage_t* islSdkSonarImageCreate(void);

/***************************************************************************************************
* Delete and free the memory for the created object.
* 
* @param inst A pointer to the object
***************************************************************************************************/
void islSdkSonarImageDelete(islSdkSonarImage_t* inst);

/***************************************************************************************************
* Set the step size to use when the passed in step size is zero
*
* @param inst A pointer to the object
* @param stepSize Step size to use when step size is zero in the iss360Ping_t struct
***************************************************************************************************/
void islSdkSonarImageSetDefaultStepSize(islSdkSonarImage_t* inst, int32_t stepSize);

/***************************************************************************************************
* Add sonar data to the helper object
* 
* @param inst A pointer to the object
* @param ping A pointer to the ping data
* @param blankRangeMm Number of millimeters to blank out at the start
***************************************************************************************************/
uint32_t islSdkSonarImageAddPingData(islSdkSonarImage_t* inst, const iss360Ping_t* ping, uint32_t blankRangeMm);

/***************************************************************************************************
* Clear a sector of sonar data
* 
* @param inst A pointer to the object
* @param startAngle Start of the sector. 0 - 12800 where 0 = 0 degrees and 12800 = 360 degrees
* @param angleSize Sector size to clear. 0 - 12800 where 0 = 0 degrees and 12800 = 360 degrees
***************************************************************************************************/
void islSdkSonarImageClearSection(islSdkSonarImage_t* inst, uint16_t startAngle, int16_t angleSize);

/***************************************************************************************************
* Render the sonar image as a 2D rectangular texture
*
* @param inst A pointer to the object
* @param image A pointer to the image buffer
* @param updateArea Area of the passed image to update. If NULLPTR then any changes since the last call are rendered 
* @param pixelArea Area of the image that was updated, can be NULLPTR
* @return TRUE if succeeded
***************************************************************************************************/
bool_t islSdkSonarImageRenderTexture(islSdkSonarImage_t* inst, sonarImageBuf_t* image, const islSdkpalette_t* palette, const sonarArea_t* updateArea, pixelArea_t* pixelArea);

/***************************************************************************************************
* Render the sonar image as a a circle or sector
*
* @param inst A pointer to the object
* @param image A pointer to the image buffer
* @param updateArea Area of the passed image to update. If NULLPTR then any changes since the last call are rendered
* @return TRUE if succeeded
***************************************************************************************************/
bool_t islSdkSonarImageRender(islSdkSonarImage_t* inst, sonarImageBuf_t* image, const islSdkpalette_t* palette, const sonarArea_t* updateArea);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
