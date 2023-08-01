#ifndef ISLSDKISD4000_H_
#define ISLSDKISD4000_H_
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

/// When an isd4000 event occurs a callback function with the signature ::sdkEventHandler_t is called. 
/// To set this callback see islSdkIsd4000SetEvent(). 
/// The {@link sdkEventHandler_t id} parameter is set to the SDK assigned id of the device and the
/// {@link sdkEventHandler_t eventId} parameter is set to one of these and this essentially defines how to cast the void* {@link sdkEventHandler_t data} parameter in the callback
typedef enum
{
	EVENT_ISD4000_ERROR,						///< An error occurred. Cast void* to sdkError_t *. { @link sdkError_t errorCode } will be one of ::isd4000ErrorCodes_t
	EVENT_ISD4000_PRESSURE_DATA,				///< New pressure and depth data has been received. Cast the void* to isd4000Pressure_t *
	EVENT_ISD4000_ORIENTATION_DATA,				///< New orientation data has been received. Cast the void* to isd4000Orientation_t *
	EVENT_ISD4000_GYRO_DATA,					///< New gyro data has been received. Cast the void* to vector3_t *
	EVENT_ISD4000_ACCEL_DATA,					///< New Accelerometer data has been received. Cast the void* to vector3_t *
	EVENT_ISD4000_MAG_DATA,						///< New Magnetometer data has been received. Cast the void* to vector3_t *
	EVENT_ISD4000_TEMPERATURE_DATA,				///< New temperature data has been received. Cast the void* to isd4000Temperature_t *
	EVENT_ISD4000_SETTINGS,						///< The settings has been received. Cast the void* to isd4000Settings_t *
	EVENT_ISD4000_SETTINGS_UPDATED,				///< Confirms if islSdkIsd4000SaveSettings() worked. Cast the void* to bool_t *. If true then settings were updated, else see error event.
	EVENT_ISD4000_CAL,							///< Calibration data has been received. Cast the void* to isd4000AhrsCal_t *
    EVENT_ISD4000_PRESSURE_CAL,                 ///< Calibration data has been received. Cast the void* to isd4000PressureCal_t *
    EVENT_ISD4000_TEMPERATURE_CAL,              ///< Calibration data has been received. Cast the void* to isd4000Temperature_t *
	EVENT_ISD4000_STRING_NAMES,					///< This is the list of hardcoded output strings avalible on the device. Cast the void* to deviceOutputStringNames_t *
	EVENT_ISD4000_SCRIPT_VARS,					///< A list of supported script varibles. Cast the void* to deviceScriptVars_t *
	EVENT_ISD4000_SCRIPT,						///< The current script on the device. Cast the void* to deviceScript_t *
	EVENT_ISD4000_SENSOR_INFO					///< Cast the void* to isd4000PressureSenorInfo_t *
} isd4000Event_t;

/// Error codes see ::EVENT_ISD4000_ERROR.
typedef enum
{
	ERROR_ISD4000_PARAM_INVALID,				///< invalid settings value
} isd4000ErrorCodes_t;

/// Controls the intervals for events
typedef struct
{
	uint32_t pressure;							///< Interval in milliseconds for generating an ::EVENT_ISD4000_PRESSURE_DATA event
	uint32_t orientation;						///< Interval in milliseconds for generating an ::EVENT_ISD4000_ORIENTATION_DATA event
	uint32_t gyro;								///< Interval in milliseconds for generating an ::EVENT_ISD4000_GYRO_DATA event
	uint32_t accel;								///< Interval in milliseconds for generating an ::EVENT_ISD4000_ACCEL_DATA event
	uint32_t mag;								///< Interval in milliseconds for generating an ::EVENT_ISD4000_MAG_DATA event
	uint32_t temperature;						///< Interval in milliseconds for generating an ::EVENT_ISD4000_TEMPERATURE_DATA event
} isd4000SensorRates_t;

/// Pressure and depth data - received on ::EVENT_ISD4000_PRESSURE_DATA event.
typedef struct
{
	uint64_t timeUs;							///< Time stamp in micro seconds of the reading since power up
	real_t barRaw;								///< Presure in Bar with no calibration
	real_t bar;									///< Presure in Bar calibrated
	real_t depthM;								///< Depth in Meters calculated from calibrated pressure
} isd4000Pressure_t;

/// Orientation data - received on ::EVENT_ISD4000_ORIENTATION_DATA event.
typedef struct
{
	uint64_t timeUs;							///< Time stamp in micro seconds of the reading since power up
	quaternion_t q;								///< Orientation information in quaternion format
	real_t magHeading;							///< Tilt compensated magnetic compass in radians
	real_t turnsCount;							///< Turns counter
} isd4000Orientation_t;

/// Temperature data - received on ::EVENT_ISD4000_TEMPERATURE_DATA event.
typedef struct
{
	real_t tempRawC;							///< Temperature in celsius with no calibration
	real_t tempC;								///< Temperature in celsius calibrated
} isd4000Temperature_t;

typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t calPointsLength;
	uint8_t verifyPointsLength;
	point_t calPoints[10];
	point_t verifyPoints[10];
	uint8_t number[32];
	uint8_t organisation[32];
	uint8_t person[32];
	uint8_t equipment[32];
	uint8_t equipmentSn[32];
	uint8_t notes[96];
} isd4000CalCert_t;

/// Calibration data - received on ::EVENT_ISD4000_CAL event.
typedef struct
{
	vector3_t gyroBias;							///< Gyro bias corrections in dps
	vector3_t accelBias;						///< Accel bias corrections in G
	vector3_t magBias;							///< Mag bias corrections in uT
	matrix3x3_t accelTransform;					///< Transformation matrix for accelerometer
	matrix3x3_t magTransform;					///< Transformation matrix for magnetometer
} isd4000AhrsCal_t;

/// Calibration data - received on ::EVENT_ISD4000_PRESSURE_CAL event.
typedef struct
{
    isd4000CalCert_t cal;
} isd4000PressureCal_t;

/// Calibration data - received on ::EVENT_ISD4000_TEMPERATURE_CAL event.
typedef struct
{
    isd4000CalCert_t cal;
    int32_t adcOffset;
} isd4000TemperatureCal_t;

/// Sensor info data - received on ::EVENT_ISD4000_SENSOR_INFO event.
typedef struct
{
	uint8_t senClass;
	uint8_t group;
	uint8_t year;
	uint8_t week;
	uint8_t bufSize;
	uint8_t status;
	uint32_t sn;
	uint16_t calYear;
	uint8_t calMonth;
	uint8_t calDay;
	real_t minPressure;
	real_t maxPressure;
} isd4000PressureSenorInfo_t;

typedef struct
{
	uint8_t strId;							///< Id of the string 0 = script
	bool_t intervalEnabled;					///< If true then autonomously output at the defined interval.
	uint32_t intervalMs;					///< Interval in milliseconds to autonomously output.
	deviceCustomStr_t interrogation;		///< Custom interrogation string
} isd4000CustomStr_t;

// Settings data - received on ::EVENT_ISD4000_SETTINGS event.
typedef struct
{
	deviceUartMode_t uartMode;				///< Serial port mode.
	uint32_t baudrate;						///< Serial port baudrate. Limits are standard bauds between 300 and 115200.
	uartParity_t parity;					///< Serial parity.
	uint8_t dataBits;						///< Serial word length 5 to 8 bits.
	uartStopBits_t stopBits;				///< Serial stop bits.
	uint8_t ahrsOptions;					///< If bit zero is 1 use inertial mode. 0 is mag slave mode
	quaternion_t orientationOffset;			///< Heading, pitch and roll offsets (or down and forward vectors) expressed as a quaternion. 
	real_t headingOffset;					///< Offset in radians to add to the heading. Typically use for magnetic declination.
	vector3_t turnsAbout;					///< A vector representing the axis which turn are measured about.
	bool_t turnsAboutEarthFrame;			///< If true the "turnAbout" vector is referenced to the earth frame. False is sensor frame.
	deviceCustomStr_t clrTurn;				///< The turns clearing string
	deviceCustomStr_t setHeading2Mag;		///< A string to set the heading to magnetometer heading
	bool_t filterPressure;					///< If true an exponential moving average filter is used to smooth data
	real_t depthOffset;						///< Offset in Meters to add to the depth.
	real_t pressureOffset;					///< Offset in Bar to add to the calibrated pressure.
	real_t latitude;						///< Latitude of the device. Used for gravity accuracy.
	deviceCustomStr_t tareStr;				///< Custom string to tare the pressure.
	deviceCustomStr_t unTareStr;			///< Custom string to remove the tare on the pressure.
	isd4000CustomStr_t strTrigger[2];
} isd4000Settings_t;

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Sets the callback for isd4000 events.
* @param callback A pointer to a function that will handle the isd4000 events listed in ::isd4000Event_t
***************************************************************************************************/
void islSdkIsd4000SetEvent(sdkEventHandler_t callback);

/***************************************************************************************************
* Poll the sensor for one or more various types of data.
* This will trigger isd4000 events with the {@link sdkEventHandler_t eventId} parameter set to one of the following
* ::EVENT_ISD4000_PRESSURE_DATA
* ::EVENT_ISD4000_ORIENTATION_DATA
* ::EVENT_ISD4000_GYRO_DATA
* ::EVENT_ISD4000_ACCEL_DATA
* ::EVENT_ISD4000_MAG_DATA
* ::EVENT_ISD4000_TEMPERATURE_DATA
*
* @param id The id of the isd4000 as returned by the SDK
* @param sensors A pointer to a struct defining which sensors to poll. Any non zero elements will be polled
***************************************************************************************************/
void islSdkIsd4000GetData(uint32_t id, const isd4000SensorRates_t* sensors);

/***************************************************************************************************
* Set the data intervals for the sensor
* This will trigger isd4000 events with the {@link sdkEventHandler_t eventId} parameter set to one of the following
* ::EVENT_ISD4000_PRESSURE_DATA
* ::EVENT_ISD4000_ORIENTATION_DATA
* ::EVENT_ISD4000_GYRO_DATA
* ::EVENT_ISD4000_ACCEL_DATA
* ::EVENT_ISD4000_MAG_DATA
* ::EVENT_ISD4000_TEMPERATURE_DATA
*
* @param id The id of the isd4000 as returned by the SDK
* @param sensors A pointer to a struct defining the intervals in milliseconds. Zero stops the data.
***************************************************************************************************/
void islSdkIsd4000SetDataIntervalsMs(uint32_t id, const isd4000SensorRates_t* sensors);

/***************************************************************************************************
* Get the isd4000 settings.
* This will trigger isd4000 events with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISD4000_SETTINGS
*
* @param id The id of the isd4000 as returned by the SDK
***************************************************************************************************/
void islSdkIsd4000GetSettings(uint32_t id);

/***************************************************************************************************
* Set and save the settings.
* This will trigger isd4000 event with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISD4000_SETTINGS_UPDATED
*
* @param id The id of the isd4000 as returned by the SDK
* @param settings A pointer to a isd4000Settings_t struct containing the settings
***************************************************************************************************/
void islSdkIsd4000SetSettings(uint32_t id, const isd4000Settings_t* settings, bool_t save);

/***************************************************************************************************
* Get the pressure calibration certificate and data
*
* @param id The id of the isd4000 as returned by the SDK
***************************************************************************************************/
void islSdkIsd4000GetPressureCal(uint32_t id);

/***************************************************************************************************
* Get the temperature calibration certificate and data
*
* @param id The id of the isd4000 as returned by the SDK
***************************************************************************************************/
void islSdkIsd4000GetTemperatureCal(uint32_t id);

/***************************************************************************************************
* Gets a list of names for each hardcoded string on the device
*
* @param id The id of the isd4000 as returned by the SDK
* @param listId the list 0 or 1
***************************************************************************************************/
void islSdkIsd4000GetStringNames(uint32_t id, uint32_t listId);

/***************************************************************************************************
* Gets a list of the supported script varibles along with a description for this device
*
* @param id The id of the isd4000 as returned by the SDK
***************************************************************************************************/
void islSdkIsd4000GetScriptVars(uint32_t id);

/***************************************************************************************************
* Gets the current running script
*
* @param id The id of the isd4000 as returned by the SDK
* @param number 0 or 1
***************************************************************************************************/
void islSdkIsd4000GetScript(uint32_t id, uint32_t number);

/***************************************************************************************************
* Sets the script to run for string output
*
* @param id The id of the isd4000 as returned by the SDK
* @param number 0 or 1
* @param name A pointer to the name
* @param script A pointer to the script code
***************************************************************************************************/
void islSdkIsd4000SetScript(uint32_t id, uint32_t number, const uint8_t* name, const uint8_t* script);

/***************************************************************************************************
* Get the pressure calibration
*
* @param id The id of the isd4000 as returned by the SDK
***************************************************************************************************/
void islSdkIsd4000GetPressureCal(uint32_t id);

/***************************************************************************************************
* Set the pressure calibration
*
* @param id The id of the isd4000 as returned by the SDK
* @param cal a pointer to a isd4000CalCert_t struct
***************************************************************************************************/
void islSdkIsd4000SetPressureCal(uint32_t id, const isd4000CalCert_t* cal);

/***************************************************************************************************
* Get the temperature calibration
*
* @param id The id of the isd4000 as returned by the SDK
***************************************************************************************************/
void islSdkIsd4000GetTemperatureCal(uint32_t id);

/***************************************************************************************************
* Set the temperature calibration
*
* @param id The id of the isd4000 as returned by the SDK
* @param cal a pointer to a isd4000CalCert_t struct
***************************************************************************************************/
void islSdkIsd4000SetTemperatureCal(uint32_t id, const isd4000CalCert_t* calCert);

/***************************************************************************************************
* For test only
*
* @param id The id of the isd4000 as returned by the SDK
***************************************************************************************************/
void islSdkIsd4000GetPressureSensorinfo(uint32_t id);

/***************************************************************************************************
* Populate an isd4000Settings_t struct with default values.
* No event occurs from this command
*
* @param settings A pointer to an isd4000Settings_t struct in which the defualt values will be written
* @return
*		 - TRUE if succeeded
*		 - FALSE if parameter was NULLPTR
***************************************************************************************************/
bool_t islSdkIsd4000GetDefaultSettings(isd4000Settings_t* settings);

/***************************************************************************************************
* Saves isd4000 settings to file.
* No event occurs from this command
*
* @param info A pointer to deviceInfo_t struct
* @param settings A pointer to an isd4000Settings_t struct containing the settings to save
* @param cal A pointer to an isd4000AhrsCal_t struct containing the calibration to save
* @param script0 A pointer to a deviceScript_t struct containing a script to save
* @param script1 A pointer to a deviceScript_t struct containing a script to save
* @param fileName A pointer to a UTF8 string holding the file path and name
* @note This generates an xml file. The \p fileName should contain the extension .xml
* @return
*		 - TRUE if succeeded
*		 - FALSE if failed
***************************************************************************************************/
bool_t islSdkIsd4000SaveSettingsToFile(const deviceInfo_t* info, const isd4000Settings_t* settings, const deviceScript_t* script0, const deviceScript_t* script1, const isd4000AhrsCal_t* cal, const isd4000PressureCal_t* pCal, const isd4000TemperatureCal_t* tCal, const str_t* fileName);

/***************************************************************************************************
* Load isd4000 settings from file.
* No event occurs from this command
*
* @param info A pointer to deviceInfo_t struct in which the file values will be written
* @param settings A pointer to an isd4000Settings_t struct in which the file values will be written
* @param script0 A pointer to a deviceScript_t struct in which the file values will be written. The name / code pointers and nameSize / codeSize must be set to a buffer in which the read data will be placed
* @param script1 A pointer to a deviceScript_t struct in which the file values will be written. The name / code pointers and nameSize / codeSize must be set to a buffer in which the read data will be placed
* @param cal A pointer to an isd4000AhrsCal_t struct in which the file values will be written
* @param fileName A pointer to a UTF8 string holding the file path and name to load
* @return
*		 - TRUE if succeeded
*		 - FALSE if failed
***************************************************************************************************/
bool_t islSdkIsd4000LoadSettingsFromFile(deviceInfo_t* info, isd4000Settings_t* settings, deviceScript_t* script0, deviceScript_t* script1, isd4000AhrsCal_t* cal, isd4000PressureCal_t* pCal, isd4000TemperatureCal_t* tCal, const str_t* fileName);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
