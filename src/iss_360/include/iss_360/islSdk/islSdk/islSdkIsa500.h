#ifndef ISLSDKISA500_H_
#define ISLSDKISA500_H_
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


/// When an isa500 event occurs a callback function with the signature ::sdkEventHandler_t is called. 
/// To set this callback see islSdkIsa500SetEvent(). 
/// The {@link sdkEventHandler_t id} parameter is set to the SDK assigned id of the device and the
/// {@link sdkEventHandler_t eventId} parameter is set to one of these and this essentially defines how to cast the void* {@link sdkEventHandler_t data} parameter in the callback
typedef enum
{
	EVENT_ISA500_ERROR,							///< An error occurred. Cast void* to sdkError_t *. { @link sdkError_t errorCode } will be one of ::isa500ErrorCodes_t
	EVENT_ISA500_ECHO_DATA,						///< New echo data has been received. Cast the void* to isa500Echoes_t *
	EVENT_ISA500_ORIENTATION_DATA,				///< New orientation data has been received. Cast the void* to isa500Orientation_t *
	EVENT_ISA500_GYRO_DATA,						///< New gyro data has been received. Cast the void* to vector3_t *
	EVENT_ISA500_ACCEL_DATA,					///< New Accelerometer data has been received. Cast the void* to vector3_t *
	EVENT_ISA500_MAG_DATA,						///< New Magnetometer data has been received. Cast the void* to vector3_t *
	EVENT_ISA500_TEMPERATURE_DATA,				///< New temperature data has been received. Cast the void* to real_t *
	EVENT_ISA500_VOLTAGE_DATA,					///< New power rail data has been received. Cast the void* to real_t *
	EVENT_ISA500_TRIGGER_DATA,					///< The TTL pin has been triggered. void* is NULLPTR
	EVENT_ISA500_SETTINGS,						///< The settings has been received. Cast the void* to isa500Settings_t *
	EVENT_ISA500_SETTINGS_UPDATED,				///< Confirms if islSdkIsa500SetSettings() worked. Cast the void* to bool_t *. If true then settings were updated, else see error event.
	EVENT_ISA500_CAL,							///< Calibration data has been received. Cast the void* to isa500Cal_t *
	EVENT_ISA500_STRING_NAMES,					///< This is the list of hardcoded output strings avalible on the device. Cast the void* to deviceOutputStringNames_t *
	EVENT_ISA500_SCRIPT_VARS,					///< A list of supported script varibles. Cast the void* to deviceScriptVars_t *
	EVENT_ISA500_SCRIPT,						///< The current script on the device. Cast the void* to deviceScript_t *
	EVENT_ISA500_PING_TEST,						///< Cast the void* to isa500TestWaveform_t *
} isa500Event_t;

/// Error codes see ::EVENT_ISA500_ERROR.
typedef enum
{
	ERROR_ISA500_PARAM_INVALID,					///< invalid settings value
} isa500ErrorCodes_t;

/// Controls the intervals for events
typedef struct
{
	uint32_t ping;								///< Interval in milliseconds for generating an ::EVENT_ISA500_ECHO_DATA event
	uint32_t orientation;						///< Interval in milliseconds for generating an ::EVENT_ISD4000_ORIENTATION_DATA event
	uint32_t gyro;								///< Interval in milliseconds for generating an ::EVENT_ISD4000_GYRO_DATA event
	uint32_t accel;								///< Interval in milliseconds for generating an ::EVENT_ISD4000_ACCEL_DATA event
	uint32_t mag;								///< Interval in milliseconds for generating an ::EVENT_ISD4000_MAG_DATA event
	uint32_t temperature;						///< Interval in milliseconds for generating an ::EVENT_ISD4000_TEMPERATURE_DATA event
	uint32_t voltage;							///< Interval in milliseconds for generating an ::EVENT_ISA500_VOLTAGE_DATA event
} isa500SensorRates_t;

/// Orientation data - received on ::EVENT_ISA500_ORIENTATION_DATA event.
typedef struct
{
	uint64_t timeUs;							///< Time stamp in micro seconds of the reading since power up
	quaternion_t q;								///< Orientation information in quaternion format
	real_t magHeading;							///< Tilt compensated magnetic compass in radians
	real_t turnsCount;							///< Turns counter
} isa500Orientation_t;

typedef struct
{
	real_t totalTof;							///< Total time of flight in seconds to the target and back
	real_t correlation;							///< How well the received echo correlates 0 to 1
	real_t signalEnergy;						///< Normalised energy level of the echo 0 to 1
} isa500Echo_t;

typedef struct
{
	uint64_t timeUs;							///< Time stamp in micro seconds since power up of the transmitsion of the ping 
	uint32_t length;							///< Number of elements in the echo array pointer to by echo
	uint32_t selectedIdx;						///< Index of the echo selected by the selection algorithm
	uint32_t totalEchoCount;					///< Total number of echos received
	isa500Echo_t* echo;							///< Pointer to the array of echos
} isa500Echoes_t;

/// Calibration data - received on ::EVENT_ISA500_CAL event.
typedef struct
{
	vector3_t gyroBias;							///< Gyro bias corrections in dps
	vector3_t accelBias;						///< Accel bias corrections in G
	vector3_t magBias;							///< Mag bias corrections in uT
	matrix3x3_t accelTransform;					///< Transformation matrix for accelerometer
	matrix3x3_t magTransform;					///< Transformation matrix for magnetometer
} isa500AhrsCal_t;

/// Test data - received on ::EVENT_ISA500_PING_TEST event.
typedef struct
{
	uint32_t totalEchoCount;
	real_t totalTof;
	real_t correlation;
	real_t signalEnergy;
	uint32_t length;
	int16_t* data;
} isa500TestWaveform_t;

typedef enum
{
	ANALOGUE_NONE = 0,
	ANALOGUE_VOLTAGE = 1,
	ANALOGUE_CURRENT = 2,
} isa500AnalogueOutMode_t;

typedef enum
{
	ECHO_FIRST = 0,
	ECHO_STRONGEST = 1,
	ECHO_TRACKING = 2,
} echoAnalyseMode_t;

typedef struct
{
	uint8_t strId;							///< Id of the string 0 = script
	bool_t intervalEnabled;					///< If true then autonomously ping and output at the defined interval.
	uint32_t intervalMs;					///< Interval in milliseconds to autonomously output.
	bool_t triggerEnabled;					///< If true the device will ping and output when trigged by the TTL input
	bool_t triggerEdge;						///< If true then the action will happen on the rising edge, false = falling edge.
	deviceCustomStr_t interrogation;		///< Custom interrogation string
} isa500CustomStr_t;

// Settings data - received on ::EVENT_ISA500_SETTINGS event.
typedef struct								// Isa500 Settings information.
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
	deviceCustomStr_t clrTurn;				///< The turns clearing string.
	deviceCustomStr_t setHeading2Mag;		///< A string to set the heading to magnetometer heading.
	uint8_t multiEchoLimit;					///< Sets the maximum multi echo limit.
	uint32_t frequency;						///< Frequency of operation in Hertz from 50000 to 700000.
	uint16_t txPulseWidthUs;				///< Length of the transmit pulse in microseconds ranging from 0 to 500.
	uint8_t txPulseAmplitude;				///< Amplitude of the transmit pulse as a percentage ranging from 1 to 100.
	echoAnalyseMode_t echoAnalyseMode;		///< Selects which echo to report back as the chosen one.
	real_t xcThreasholdLow;					///< Sets a lower limit on the quality of the return pulse. This ensures resilience to false echos. Value ranges from 0 to 1 default is 0.4.
	real_t xcThreasholdHigh;				///< When the return signal level drops bellow this value the end of an echo pulse is realised. Value ranges from 0 to 1 default is 0.5.
	real_t energyThreashold;				///< Minimum enery an echo must have to be reported. Range is 0 to 1
	real_t speedOfSound;					///< Speed of sound in meters per second.
	real_t minRange;						///< Minimum range in meters.
	real_t maxRange;						///< Maximum range in meters. Upper limit is 300.
	real_t distanceOffset;					///< Offset + or - in meters to add to the final reading.
	bool_t useTiltCorrection;				///< If true the echo range to target will be trigonometrically corrected for a beam perpendicular to the target.
	bool_t useMaxValueOnNoReturn;			///< If no echo is detected then use the maximum range value as the reading for outputted strings.
	isa500AnalogueOutMode_t anaMode;		///< Mode of the analogue output.
	real_t aOutMinRange;					///< Value in meters. "aOutMinRange" and "aOutMinVal" define a point. e.g 3 meters = 3 volt.
	real_t aOutMaxRange;					///< Value in meters. "aOutMaxRange" and "aOutMaxVal" define another point. e.g 10 meters = 10 volt. These 2 points define a straight line which relates range to output value.
	real_t aOutMinVal;						///< Volts or milliamps depending on mode.
	real_t aOutMaxVal;						///< Volts or milliamps depending on mode.
	isa500CustomStr_t strTrigger[2];
} isa500Settings_t;

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Sets the callback for isa500 events.
* @param callback A pointer to a function that will handle the isa500 events listed in ::isa500Event_t
***************************************************************************************************/
void islSdkIsa500SetEvent(sdkEventHandler_t callback);

/***************************************************************************************************
* Poll the sensor for one or more various types of data.
* This will trigger isa500 events with the {@link sdkEventHandler_t eventId} parameter set to one of the following
* ::EVENT_ISA500_ECHO_DATA
* ::EVENT_ISA500_ORIENTATION_DATA
* ::EVENT_ISA500_GYRO_DATA
* ::EVENT_ISA500_ACCEL_DATA
* ::EVENT_ISA500_MAG_DATA
* ::EVENT_ISA500_TEMPERATURE_DATA
* ::EVENT_ISA500_VOLTAGE_DATA
*
* @param id The id of the isa500 as returned by the SDK
* @param sensors A pointer to a struct defining which sensors to poll. Any non zero elements will be polled
***************************************************************************************************/
void islSdkIsa500GetData(uint32_t id, const isa500SensorRates_t* sensors);

/***************************************************************************************************
* Set the data intervals for the sensor
* This will trigger isa500 events with the {@link sdkEventHandler_t eventId} parameter set to one of the following
* ::EVENT_ISA500_ECHO_DATA
* ::EVENT_ISA500_ORIENTATION_DATA
* ::EVENT_ISA500_GYRO_DATA
* ::EVENT_ISA500_ACCEL_DATA
* ::EVENT_ISA500_MAG_DATA
* ::EVENT_ISA500_TEMPERATURE_DATA
* ::EVENT_ISA500_VOLTAGE_DATA
*
* @param id The id of the isa500 as returned by the SDK
* @param sensors A pointer to a struct defining the intervals in milliseconds. Zero stops the data.
***************************************************************************************************/
void islSdkIsa500SetDataIntervalsMs(uint32_t id, const isa500SensorRates_t* sensors);

/***************************************************************************************************
* Get the isa500 settings.
* This will trigger isa500 events with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISA500_SETTINGS
*
* @param id The id of the isa500 as returned by the SDK
***************************************************************************************************/
void islSdkIsa500GetSettings(uint32_t id);

/***************************************************************************************************
* Set and save the settings.
* This will trigger isa500 event with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISA500_SETTINGS_UPDATED
*
* @param id The id of the isa500 as returned by the SDK
* @param settings A pointer to a isa500Settings_t struct containing the settings
***************************************************************************************************/
void islSdkIsa500SetSettings(uint32_t id, const isa500Settings_t* settings, bool_t save);

/***************************************************************************************************
* Gets a list of names for each hardcoded string on the device
*
* @param id The id of the isa500 as returned by the SDK
* @param listId the list 0 or 1
***************************************************************************************************/
void islSdkIsa500GetStringNames(uint32_t id, uint32_t listId);

/***************************************************************************************************
* Gets a list of the supported script varibles along with a description for this device
*
* @param id The id of the isa500 as returned by the SDK
***************************************************************************************************/
void islSdkIsa500GetScriptVars(uint32_t id);

/***************************************************************************************************
* Gets the current running script
*
* @param id The id of the isa500 as returned by the SDK
* @param number 0 or 1
***************************************************************************************************/
void islSdkIsa500GetScript(uint32_t id, uint32_t number);

/***************************************************************************************************
* Sets the script to run for string output
*
* @param id The id of the isa500 as returned by the SDK
* @param number 0 or 1
* @param name A pointer to the name
* @param script A pointer to the script code
***************************************************************************************************/
void islSdkIsa500SetScript(uint32_t id, uint32_t number, const uint8_t* name, const uint8_t* script);

/***************************************************************************************************
* For test only. Set the TVG DAC
*
* @param id The id of the isa500 as returned by the SDK
* @param voltage 0.1 to 1.1
***************************************************************************************************/
void islSdkIsa500SetTvgDac(uint32_t id, real_t voltage);

/***************************************************************************************************
* For test only. Sets the analogue output
*
* @param id The id of the isa500 as returned by the SDK
* @param mode
* @param value
***************************************************************************************************/
void islSdkIsa500SetAnalogueOut(uint32_t id, isa500AnalogueOutMode_t mode, real_t value);

/***************************************************************************************************
* For test only. Performs a ping loopback
*
* @param id The id of the isa500 as returned by the SDK
***************************************************************************************************/
void islSdkIsa500TestPing(uint32_t id);

/***************************************************************************************************
* Populate an isa500Settings_t struct with default values.
* No event occurs from this command
*
* @param settings A pointer to an isa500Settings_t struct in which the defualt values will be written
* @return
*		 - TRUE if succeeded
*		 - FALSE if parameter was NULLPTR
***************************************************************************************************/
bool_t islSdkIsa500GetDefaultSettings(isa500Settings_t* settings);

/***************************************************************************************************
* Saves isa500 settings to file.
* No event occurs from this command
*
* @param info A pointer to deviceInfo_t struct
* @param settings A pointer to an isa500Settings_t struct containing the settings to save
* @param cal A pointer to an isa500Cal_t struct containing the calibration to save
* @param script0 A pointer to a deviceScript_t struct containing a script to save
* @param script1 A pointer to a deviceScript_t struct containing a script to save
* @param fileName A pointer to a UTF8 string holding the file path and name
* @note This generates an xml file. The \p fileName should contain the extension .xml
* @return
*		 - TRUE if succeeded
*		 - FALSE if failed
***************************************************************************************************/
bool_t islSdkIsa500SaveSettingsToFile(const deviceInfo_t* info, const isa500Settings_t* settings, const deviceScript_t* script0, const deviceScript_t* sctrip1, const isa500AhrsCal_t* cal, const str_t* fileName);

/***************************************************************************************************
* Load isa500 settings from file.
* No event occurs from this command
*
* @param info A pointer to deviceInfo_t struct in which the file values will be written
* @param settings A pointer to an isa500Settings_t struct in which the file values will be written
* @param script0 A pointer to a deviceScript_t struct in which the file values will be written. The name / code pointers and nameSize / codeSize must be set to a buffer in which the read data will be placed 
* @param script1 A pointer to a deviceScript_t struct in which the file values will be written. The name / code pointers and nameSize / codeSize must be set to a buffer in which the read data will be placed 
* @param cal A pointer to an isa500Cal_t struct in which the file values will be written
* @param fileName A pointer to a UTF8 string holding the file path and name to load
* @return
*		 - TRUE if succeeded
*		 - FALSE if failed
***************************************************************************************************/
bool_t islSdkIsa500LoadSettingsFromFile(deviceInfo_t* info, isa500Settings_t* settings, deviceScript_t* script0, deviceScript_t* script1, isa500AhrsCal_t* cal, const str_t* fileName);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
