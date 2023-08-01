#ifndef ISLSDKISM3D_H_
#define ISLSDKISM3D_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "platform/uart.h"
#include "islSdkDevice.h"
#include "islSdkVector.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

/// When an ism3d event occurs a callback function with the signature ::sdkEventHandler_t is called. 
/// To set this callback see islSdkIsm3dSetEvent(). 
/// The {@link sdkEventHandler_t id} parameter is set to the SDK assigned id of the device and the
/// {@link sdkEventHandler_t eventId} parameter is set to one of these and this essentially defines how to cast the void* {@link sdkEventHandler_t data} parameter in the callback
typedef enum
{
	EVENT_ISM3D_ERROR,							///< An error occurred. Cast void* to sdkError_t *. { @link sdkError_t errorCode } will be one of ::ism3dErrorCodes_t
	EVENT_ISM3D_ORIENTATION_DATA,				///< New orientation data has been received. Cast the void* to ism3dOrientation_t *
	EVENT_ISM3D_GYRO_DATA,						///< New gyro data has been received. Cast the void* to vector3_t *
	EVENT_ISM3D_ACCEL_DATA,						///< New Accelerometer data has been received. Cast the void* to vector3_t *
	EVENT_ISM3D_MAG_DATA,						///< New Magnetometer data has been received. Cast the void* to vector3_t *
	EVENT_ISM3D_SETTINGS,						///< The settings has been received. Cast the void* to ism3dSettings_t *
	EVENT_ISM3D_SETTINGS_UPDATED,				///< Confirms if islSdkIsm3dSaveSettings() worked. Cast the void* to bool_t *. If true then settings were updated, else see error event.
	EVENT_ISM3D_CAL,							///< Calibration data has been received. Cast the void* to ism3dAhrsCal_t *
	EVENT_ISM3D_STRING_NAMES,					///< This is the list of hardcoded output strings avalible on the device. Cast the void* to deviceOutputStringNames_t *
	EVENT_ISM3D_SCRIPT_VARS,					///< A list of supported script varibles. Cast the void* to deviceScriptVars_t *
	EVENT_ISM3D_SCRIPT,							///< The current script on the device. Cast the void* to deviceScript_t *
	EVENT_ISM3D_SERIAL_TEST,
} ism3dEvent_t;

/// Error codes see ::EVENT_ISM3D_ERROR.
typedef enum
{
	ERROR_ISM3D_PARAM_INVALID,					///< invalid settings value
} ism3dErrorCodes_t;

/// Controls the intervals for events.
typedef struct
{
	uint32_t orientation;				///< Interval in milliseconds for generating an ::EVENT_ISM3D_ORIENTATION_DATA event
	uint32_t gyro;						///< Interval in milliseconds for generating an ::EVENT_ISM3D_GYRO_DATA event
	uint32_t accel;						///< Interval in milliseconds for generating an ::EVENT_ISM3D_ACCEL_DATA event
	uint32_t mag;						///< Interval in milliseconds for generating an ::EVENT_ISM3D_MAG_DATA event
} ism3dSensorRates_t;

/// Orientation data - received on ::EVENT_ISM3D_ORIENTATION_DATA event.
typedef struct
{
	uint64_t timeUs;					///< Time stamp in micro seconds of the reading since power up
	quaternion_t q;						///< Orientation information in quaternion format
	real_t magHeading;					///< Tilt compensated magnetic compass in radians
	real_t turnsCount;					///< Turns counter
} ism3dOrientation_t;

/// Calibration data - received on ::EVENT_ISM3D_CAL event.
typedef struct
{
	vector3_t gyroBias;					///< Gyro bias corrections in dps
	vector3_t accelBias;				///< Accel bias corrections in G
	vector3_t gyroBiasSec;				///< Backup Gyro bias corrections in dps
	vector3_t accelBiasSec;				///< Backup Accel bias corrections in G;
	vector3_t magBias;					///< Mag bias corrections in uT
	matrix3x3_t accelTransform;			///< Transformation matrix for accelerometer
	matrix3x3_t accelTransformSec;		///< Transformation matrix for backup accelerometer
	matrix3x3_t magTransform;			///< Transformation matrix for magnetometer
} ism3dAhrsCal_t;

typedef struct
{
	uint8_t strId;							///< Id of the string 0 = script
	bool_t intervalEnabled;					///< If true then autonomously output at the defined interval.
	uint32_t intervalMs;					///< Interval in milliseconds to autonomously output.
	deviceCustomStr_t interrogation;		///< Custom interrogation string
} ism3dCustomStr_t;

/// Settings data - received on ::EVENT_ISM3D_SETTINGS event.
typedef struct
{
	deviceUartMode_t uartMode;				///< Serial port mode
	uint32_t baudrate;						///< Serial port baudrate. Limits are standard bauds between 9600 and 921600
	uartParity_t parity;					///< Serial parity
	uint8_t dataBits;						///< Serial word length 5 to 8 bits
	uartStopBits_t stopBits;				///< Serial stop bits
	uint8_t ahrsOptions;					///< If bit zero is 1 use inertial mode. 0 is mag slave mode
	quaternion_t orientationOffset;			///< Heading, pitch and roll offsets (or down and forward vectors) expressed as a quaternion. 
	real_t headingOffset;					///< Offset in radians to add to the heading. Typically use for magnetic declination.
	vector3_t turnsAbout;					///< A vector representing the axis which turn are measured about.
	bool_t turnsAboutEarthFrame;			///< If true the "turnAbout" vector is referenced to the earth frame. False is sensor frame.
	deviceCustomStr_t clrTurn;				///< The turns clearing string
	deviceCustomStr_t setHeading2Mag;		///< A string to set the heading to magnetometer heading
	ism3dCustomStr_t strTrigger;
} ism3dSettings_t;

typedef struct
{
	bool_t passed;
	uint32_t size;
	uint8_t* data;
} ism3dSerialTest_t;

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Sets the callback for ism3d events.
* @param callback A pointer to a function that will handle the ism3d events listed in ::ism3dEvent_t
***************************************************************************************************/
void islSdkIsm3dSetEvent(sdkEventHandler_t callback);

/***************************************************************************************************
* Poll the sensor for one or more various types of data.
* This will trigger ism3d events with the {@link sdkEventHandler_t eventId} parameter set to one of the following
* ::EVENT_ISM3D_ORIENTATION_DATA
* ::EVENT_ISM3D_GYRO_DATA
* ::EVENT_ISM3D_ACCEL_DATA
* ::EVENT_ISM3D_MAG_DATA
*
* @param id The id of the ism3d as returned by the SDK
* @param sensors A pointer to a struct defining which sensors to poll. Any non zero elements will be polled
***************************************************************************************************/
void islSdkIsm3dGetData(uint32_t id, const ism3dSensorRates_t* sensors);

/***************************************************************************************************
* Set the data intervals for the sensor
* This will trigger ism3d events with the {@link sdkEventHandler_t eventId} parameter set to one of the following
* ::EVENT_ISM3D_ORIENTATION_DATA
* ::EVENT_ISM3D_GYRO_DATA
* ::EVENT_ISM3D_ACCEL_DATA
* ::EVENT_ISM3D_MAG_DATA
*
* @param id The id of the ism3d as returned by the SDK
* @param sensors A pointer to a struct defining the intervals in milliseconds. Zero stops the data.
***************************************************************************************************/
void islSdkIsm3dSetDataIntervalsMs(uint32_t id, const ism3dSensorRates_t* sensors);

/***************************************************************************************************
* Get the ism3d settings.
* This will trigger ism3d events with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISM3D_SETTINGS
*
* @param id The id of the ism3d as returned by the SDK
***************************************************************************************************/
void islSdkIsm3dGetSettings(uint32_t id);

/***************************************************************************************************
* Set and save the settings.
* This will trigger ism3d event with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISM3D_SETTINGS_UPDATED
*
* @param id The id of the ism3d as returned by the SDK
* @param settings A pointer to a ism3dSettings_t struct containing the settings
***************************************************************************************************/
void islSdkIsm3dSetSettings(uint32_t id, const ism3dSettings_t* settings, bool_t save);

/***************************************************************************************************
* Gets a list of names for each hardcoded string on the device
*
* @param id The id of the ism3d as returned by the SDK
* @param listId the list 0 or 1
***************************************************************************************************/
void islSdkIsm3dGetStringNames(uint32_t id, uint32_t listId);

/***************************************************************************************************
* Gets a list of the supported script varibles along with a description for this device
*
* @param id The id of the ism3d as returned by the SDK
***************************************************************************************************/
void islSdkIsm3dGetScriptVars(uint32_t id);

/***************************************************************************************************
* Gets the current running script
*
* @param id The id of the ism3d as returned by the SDK
* @param number Must be 0
***************************************************************************************************/
void islSdkIsm3dGetScript(uint32_t id, uint32_t number);

/***************************************************************************************************
* Sets the script to run for string output
*
* @param id The id of the ism3d as returned by the SDK
* @param number Must be 0
* @param name A pointer to the name
* @param script A pointer to the script code
***************************************************************************************************/
void islSdkIsm3dSetScript(uint32_t id, uint32_t number, const uint8_t* name, const uint8_t* script);

/***************************************************************************************************
* Populate an ism3dSettings_t struct with default values.
* No event occurs from this command
*
* @param settings A pointer to an ism3dSettings_t struct in which the defualt values will be written
* @return
*		 - TRUE if succeeded
*		 - FALSE if parameter was NULLPTR
***************************************************************************************************/
bool_t islSdkIsm3dGetDefaultSettings(ism3dSettings_t* settings);

/***************************************************************************************************
* Saves ism3d settings to file.
* No event occurs from this command
*
* @param info A pointer to deviceInfo_t struct
* @param settings A pointer to an ism3dSettings_t struct containing the settings to save
* @param cal A pointer to an ism3dAhrsCal_t struct containing the calibration to save
* @param script0 A pointer to a deviceScript_t struct containing a script to save
* @param fileName A pointer to a UTF8 string holding the file path and name
* @note This generates an xml file. The \p fileName should contain the extension .xml
* @return
*		 - TRUE if succeeded
*		 - FALSE if failed
***************************************************************************************************/
bool_t islSdkIsm3dSaveSettingsToFile(const deviceInfo_t* info, const ism3dSettings_t* settings, const deviceScript_t* script0, const ism3dAhrsCal_t* cal, const str_t* fileName);

/***************************************************************************************************
* Load ism3d settings from file.
* No event occurs from this command
*
* @param info A pointer to deviceInfo_t struct in which the file values will be written
* @param settings A pointer to an ism3dSettings_t struct in which the file values will be written
* @param script0 A pointer to a deviceScript_t struct in which the file values will be written. The name / code pointers and nameSize / codeSize must be set to a buffer in which the read data will be placed
* @param cal A pointer to an ism3dAhrsCal_t struct in which the file values will be written
* @param fileName A pointer to a UTF8 string holding the file path and name to load
* @return
*		 - TRUE if succeeded
*		 - FALSE if failed
***************************************************************************************************/
bool_t islSdkIsm3dLoadSettingsFromFile(deviceInfo_t* info, ism3dSettings_t* settings, deviceScript_t* script0, ism3dAhrsCal_t* cal, const str_t* fileName);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
