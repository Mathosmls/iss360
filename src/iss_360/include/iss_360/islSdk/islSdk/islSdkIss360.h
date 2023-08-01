#ifndef ISLSDKISS360_H_
#define ISLSDKISS360_H_
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* @file This module contains definitions and functions for managing a ISS360 device (sonar)
***************************************************************************************************/

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "islSdkDevice.h"
#include "islSdkVector.h"

//--------------------------------------- Public Constants -----------------------------------------

#define islSdkIss360ConvertHeadAngleToDeg(a) ((real_t)(a) * (real_t)(360.0 / 12800.0))					///< Converts transducer angle units to degrees
#define islSdkIss360ConvertDegToHeadAngle(a) ((uint32_t)((real_t)(a) * (real_t)(12800.0 / 360.0)))		///< Converts degrees to transducer angle units

//----------------------------------------- Public Types ------------------------------------------

/// When an iss360 event occurs a callback function with the signature ::sdkEventHandler_t is called. 
/// To set this callback see islSdkIss360SetEvent(). 
/// The {@link sdkEventHandler_t id} parameter is set to the SDK assigned id of the device and the
/// {@link sdkEventHandler_t eventId} parameter is set to one of these and this essentially defines how to cast the void* {@link sdkEventHandler_t data} parameter in the callback
typedef enum
{
	EVENT_ISS360_ERROR,							///< An error occurred. Cast void* to sdkError_t *. { @link sdkError_t errorCode } will be one of ::iss360ErrorCodes_t
	EVENT_ISS360_ORIENTATION_DATA,				///< New orientation data has been received. Cast the void* to iss360Orientation_t *
	EVENT_ISS360_GYRO_DATA,						///< New gyro data has been received. Cast the void* to vector3_t *
	EVENT_ISS360_ACCEL_DATA,					///< New Accelerometer data has been received. Cast the void* to vector3_t *
	EVENT_ISS360_MAG_DATA,						///< New Magnetometer data has been received. Cast the void* to vector3_t *
	EVENT_ISS360_SETTINGS,						///< The settings has been received. Cast the void* to iss360Settings_t *
	EVENT_ISS360_SYSTEM_SETTINGS_UPDATED,		///< Confirms that islSdkIss360SetSystemSettings() worked. Cast the void* to bool_t *. If bool is true then settings were updated, else see error event.
	EVENT_ISS360_ACOUSTIC_SETTINGS_UPDATED,		///< Confirms that islSdkIss360SetAcousticSettings() worked. Cast the void* to bool_t *. If bool is true then settings were updated, else see error event.
	EVENT_ISS360_SETUP_SETTINGS_UPDATED,		///< Confirms that islSdkIss360SetSetupSettings() worked. Cast the void* to bool_t *. If true then settings were updated, else see error event.
	EVENT_ISS360_CAL,							///< Calibration data has been received. Cast the void* to iss360AhrsCal_t *
	EVENT_ISS360_HOMED,							///< The ISS360 head has finished homeing. Cast the void* to iss360HeadHome_t *
	EVENT_ISS360_PING_DATA,						///< New ping data received. Cast void* to iss360Ping_t *
    EVENT_ISS360_ADC_DATA,                      ///< New ping ADC data received. Cast void* to iss360AdcData_t *
	EVENT_ISS360_BUF_DATA,						///< ADC or cross correlation waveform .Cast void* to iss360bufData_t*
	EVENT_ISS360_LOOPBACK_DATA,					///< A new iss360loopBackData_t struct has been received. Cast the void* to iss360loopBackData_t *
	EVENT_ISS360_TDR_TEST,						///< A new iss360EthernetTdrTest_t struct has been received. Cast the void* to iss360EthernetTdrTest_t *
	EVENT_ISS360_CPU_TEMP_PWR, 					///< A new cpuPwrTemp_t struct has been received. Cast the void* to cpuPwrTemp_t *
} iss360Event_t;

/// Error codes see ::EVENT_ISS360_ERROR.
typedef enum
{
	ERROR_ISS360_PARAM_INVALID,         ///< invalid settings value
} iss360ErrorCodes_t;

/// Device system settings.
typedef struct
{
	uint8_t macAddress[6];				///< Mac Address of the ethernet interface. Read only, setting has no effect
	deviceUartMode_t uartMode;			///< Serial port mode
	uint32_t baudrate;					///< Serial port baudrate. Limits are standard bauds between 9600 and 921600
	uint32_t ipAddress;					///< IPv4 address. 192.168.1.200 = 0xc801A8c0
	uint32_t netmask;					///< IPv4 netmask. 255.255.255.0 = 0x00ffffff
	uint32_t gateway;					///< IPv4 gateway. 192.168.1.1 = 0x0101A8c0
	uint16_t port;                      ///< Port the device listens on and transmits from
	devicePhyPortMode_t phyPortMode;	///< Ethernet connection mode
	devicePhyMdixMode_t phyMdixMode;	///< Ethernet TX/RX swapping mode
	bool_t useDhcp;						///< If true device will request an IP address from the DHCP server
	bool_t invertHeadDirection;			///< If true the head direction is swapped
	uint8_t ahrsOptions;				///< If bit zero is 1 use inertial mode. 0 is mag slave mode
	quaternion_t orientationOffset;		///< Heading, pitch and roll offsets (or down and forward vectors) expressed as a quaternion.
	real_t headingOffset;				///< Offset in radians to add to the heading. Typically use for magnetic declination.
	vector3_t turnsAbout;				///< A vector representing the axis which turn are measured about.
	bool_t turnsAboutEarthFrame;		///< If true the "turnAbout" vector is referenced to the earth frame. False is sensor frame.
	point_t tvgPoints[9];				///< Defines the shape of the TVG curve.
} iss360SystemSettings_t;

/// Device acoustic settings.
typedef struct
{
	uint32_t txStartFrequency;			///< Transmit pulse start frequency in hertz. limits are 200000 to 1000000. Hardware is designed for 600000 to 800000
	uint32_t txEndFrequency;			///< Transmit pulse end frequency in hertz. limits are 200000 to 1000000. Hardware is designed for 600000 to 800000
	uint16_t txPulseWidthUs;			///< Transmit pulse length in micro seconds. limits are 0 to 409 when \p highSampleRate is false and 0 to 204 when \p highSampleRate is true
	uint8_t txPulseAmplitude;			///< Transmit pulse amplitude as a percent 0% to 100%
	bool_t highSampleRate;				///< If true the ADC sample rate is 5 MHz else it's 2.5 MHz
} iss360AcousticSettings_t;

/// Device operational settings.
typedef struct
{
	real_t digitalGain;					///< Digital gain for the image data as a simple multiplier factor. limits 1 to 1000
	real_t speedOfSound;				///< Speed of sound in meters per second. limits 1000 to 2500
	uint32_t maxRangeMm;				///< Listen for echos up until this range in millimeters. limts 0 to 200000
	uint32_t minRangeMm;				///< Start listening for echos after this range in millimeters limts 0 to 200000
	int32_t stepSize;					///< Angle the tranducer head should move between pings in units of 12800th. Positive values turn clockwise, negative anticlockwise. limits -6399 to 6399
	uint32_t sectorStart;				///< Start angle of the sector. limmts 0 to 12799
	uint32_t sectorSize;				///< Size of the sector. limits 0 to 12800
	bool_t flybackMode;					///< If true the transducer head returns back to either the \p sectorStart position when \p stepSize is positive, or s\p sectorStart + \p sectorSize when \p stepSize is negative
	uint16_t imageDataPoint;			///< Number of data points per ping between the range set by minRangeMm and maxRangeMm. limits 20 to 4096
	bool_t data8Bit;					///< TRUE = 8-bit data, FALSE = 16-bit data
} iss360SetupSettings_t;

/// All device settings - received on ::EVENT_ISS360_SETTINGS event.
typedef struct
{
	iss360SystemSettings_t system;
	iss360AcousticSettings_t acoustic;
	iss360SetupSettings_t setup;
} iss360Settings_t;

/// Ping image data - received on ::EVENT_ISS360_PING_DATA event
typedef struct
{
	uint16_t angle;						///< Angle the data was aquired at in units of 12800th. 360 degrees = a value of 12800
    int32_t stepSize;                   ///< The step size setting at the time this data was aquired
    uint32_t minRangeMm;				///< Start distance of the data in millimeters, \p data[0] is aquired at this range
	uint32_t maxRangeMm;				///< Final distance of the data in millimeters, \p data[dataCount-1] is aquired at this range
	uint32_t dataCount;					///< Number of elements in the data array
	uint16_t* data;						///< Pointer to array of image data for a ping
} iss360Ping_t;

/// Ping image data - received on ::EVENT_ISS360_ADC_DATA event
typedef struct
{
	uint16_t angle;						///< Angle the data was aquired at in units of 12800th. 360 degrees = a value of 12800
	int32_t stepSize;                   ///< The step size setting at the time this data was aquired
	uint32_t minRangeMm;				///< Start distance of the data in millimeters, \p data[0] is aquired at this range
	uint32_t maxRangeMm;				///< Final distance of the data in millimeters, \p data[dataCount-1] is aquired at this range
	uint32_t dataCount;					///< Number of elements in the data array
	int16_t* data;						///< Pointer to array of adc data for a ping
} iss360AdcPingData_t;

/// Controls the intervals for events.
typedef struct
{
	uint32_t orientation;				///< Interval in milliseconds for generating an ::EVENT_ISS360_ORIENTATION_DATA event
	uint32_t gyro;						///< Interval in milliseconds for generating an ::EVENT_ISS360_GYRO_DATA event
	uint32_t accel;						///< Interval in milliseconds for generating an ::EVENT_ISS360_ACCEL_DATA event
	uint32_t mag;						///< Interval in milliseconds for generating an ::EVENT_ISS360_MAG_DATA event
	uint32_t voltageAndTemp;			///< Interval in milliseconds for generating an ::EVENT_ISS360_CPU_TEMP_PWR event
} iss360SensorRates_t;

/// Orientation data - received on ::EVENT_ISS360_ORIENTATION_DATA event.
typedef struct
{
	quaternion_t q;						///< Orientation information in quaternion format
	real_t magHeading;					///< Tilt compensated magnetic compass in radians
	real_t turnsCount;					///< Turns counter
} iss360Orientation_t;

/// Calibration data - received on ::EVENT_ISS360_CAL event.
typedef struct
{
	vector3_t gyroBias;					///< Gyro bias corrections in dps
	vector3_t accelBias;				///< Accel bias corrections in G
	vector3_t magBias;					///< Mag bias corrections in uT
	matrix3x3_t accelTransform;			///< Transformation matrix for accelerometer
	matrix3x3_t magTransform;			///< Transformation matrix for magnetometer
} iss360AhrsCal_t;

/// Power and temperature data - received on ::EVENT_ISS360_CPU_TEMP_PWR event.
typedef struct
{
	real_t core1V0;
	real_t aux1V8;
	real_t ddr1V35;
	real_t cpuTemperature;
	real_t auxTemperature;
} cpuPwrTemp_t;

typedef enum
{
	ISS360_HOMED_OK,
	ISS360_HOMED_ERR_E1_E2,
	ISS360_HOMED_ERR_E2,
	ISS360_HOMED_ERR_E1,
	ISS360_HOMED_ERR,
} iss360HeadHomeState_t;

/// Head index data - received on ::EVENT_ISS360_HOMED event.
typedef struct
{
	iss360HeadHomeState_t state;
	int16_t dif;
	uint16_t edge1Idx;
	bool_t edge1Level;
	uint16_t edge2Idx;
	bool_t edge2Level;
} iss360HeadHome_t;

/// Raw ADC and cross correlation buffer data. For test and R&D - received on ::EVENT_ISS360_BUF_DATA event.
typedef struct
{
	uint32_t index;						///< Array index of this data
	uint32_t count;						///< Number of elements pointed to by data
	uint8_t bufId;						///< 0 = ADC buffer, 1 = cross correlation buffer
	int16_t* data;						///< The buffer data
} iss360bufData_t;

/// System testing - received on ::EVENT_ISS360_LOOPBACK_DATA event.
typedef struct
{
	uint32_t adcDataCount;				///< Number of elements pointed to by adcData
	uint32_t xcDataCount;				///< Number of elements pointed to by xcData
	int16_t* adcData;					///< Adc buffer data
	int16_t* xcData;					///< Cross correlated buffer data
} iss360loopBackData_t;

typedef struct
{
	int32_t distance;
	int32_t amplitude;
} tdrReading_t;

typedef struct
{
	bool_t testValid;
	uint32_t txCount;
	uint32_t rxCount;
	bool_t rxShortedToTx;
	bool_t txShortedToRx;
	bool_t txCountExceeded;
	bool_t rxCountExceeded;
	tdrReading_t txReadings[5];
	tdrReading_t rxReadings[5];
} iss360EthernetTdrTest_t;

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Sets the callback for iss360 events.
* @param callback A pointer to a function that will handle the iss360 events listed in ::iss360Event_t
***************************************************************************************************/
void islSdkIss360SetEvent(sdkEventHandler_t callback);

/***************************************************************************************************
* Start or stop the sonar acquiring data.
* This will trigger iss360 events with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISS360_PING_DATA
*
* @param id The id of the iss360 as returned by the SDK
***************************************************************************************************/
void islSdkIss360StartStop(uint32_t id, bool_t run);

/***************************************************************************************************
* Set sonar options
*
* @param sendAdcData If true the sonar will send raw ADC data for every ping
***************************************************************************************************/
void islSdkIss360SetOptions(uint32_t id, bool_t sendAdcData);

/***************************************************************************************************
* Poll the sensor for one or more various types of data.
* This will trigger iss360 events with the {@link sdkEventHandler_t eventId} parameter set to one of the following
* ::EVENT_ISS360_ORIENTATION_DATA
* ::EVENT_ISS360_GYRO_DATA
* ::EVENT_ISS360_ACCEL_DATA
* ::EVENT_ISS360_MAG_DATA
* ::EVENT_ISS360_CPU_TEMP_PWR
* 
* @param id The id of the iss360 as returned by the SDK
* @param sensors A pointer to a struct defining which sensors to poll. Any non zero elements will be polled
***************************************************************************************************/
void islSdkIss360GetData(uint32_t id, const iss360SensorRates_t* sensors);

/***************************************************************************************************
* Set the data intervals for the sensor
* This will trigger iss360 events with the {@link sdkEventHandler_t eventId} parameter set to one of the following
* ::EVENT_ISS360_ORIENTATION_DATA
* ::EVENT_ISS360_GYRO_DATA
* ::EVENT_ISS360_ACCEL_DATA
* ::EVENT_ISS360_MAG_DATA
* ::EVENT_ISS360_CPU_TEMP_PWR
*
* @param id The id of the iss360 as returned by the SDK
* @param sensors A pointer to a struct defining the intervals in milliseconds. Zero stops the data.
***************************************************************************************************/
void islSdkIss360SetDataIntervalsMs(uint32_t id, const iss360SensorRates_t* sensors);

/***************************************************************************************************
* Get the iss360 settings.
* This will trigger iss360 events with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISS360_SETTINGS
*
* @param id The id of the iss360 as returned by the SDK
***************************************************************************************************/
void islSdkIss360GetSettings(uint32_t id);

/***************************************************************************************************
* Set and save the system settings.
* This will trigger iss360 event with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISS360_SYSTEM_SETTINGS_UPDATED
*
* @param id The id of the iss360 as returned by the SDK
* @param settings A pointer to a iss360SystemSettings_t struct containing the settings
***************************************************************************************************/
void islSdkIss360SetSystemSettings(uint32_t id, const iss360SystemSettings_t* settings, bool_t save);

/***************************************************************************************************
* Set the acoustic settings.
* This will trigger an iss360 event with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISS360_ACOUSTIC_SETTINGS_UPDATED
*
* @param id The id of the iss360 as returned by the SDK
* @param settings A pointer to a iss360AcousticSettings_t struct containing the settings
***************************************************************************************************/
void islSdkIss360SetAcousticSettings(uint32_t id, const iss360AcousticSettings_t* settings, bool_t save);

/***************************************************************************************************
* Set the setup settings.
* This will trigger an iss360 event with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISS360_SETUP_SETTINGS_UPDATED
*
* @param id The id of the iss360 as returned by the SDK
* @param settings A pointer to a iss360SetupSettings_t struct containing the settings
***************************************************************************************************/
void islSdkIss360SetSetupSettings(uint32_t id, const iss360SetupSettings_t* settings, bool_t save);

/***************************************************************************************************
* Set the transducer head to the 0 degree position.
* No event occurs from this command
*
* @param id The id of the iss360 as returned by the SDK
* @param reacquire Rescan the head index markings and home
***************************************************************************************************/
void islSdkIss360HomeHead(uint32_t id, bool_t reacquire);

/***************************************************************************************************
* Enable the iss360 to generate test image data.
* The test image is a checkered pattern with the 2 values 0x4000 and 0xC000. No event occurs from this command
*
* @param id The id of the iss360 as returned by the SDK
* @param enabled If true image data is set to the test pattern
***************************************************************************************************/
void islSdkIss360EnableTestImage(uint32_t id, bool_t enabled);

/***************************************************************************************************
* Perform a time-domain reflectometer test on the ethernet interface
*
* @param id The id of the iss360 as returned by the SDK
***************************************************************************************************/
void islSdkIss360EthernetTdrTest(uint32_t id);

/***************************************************************************************************
* Populate an iss360Settings_t struct with default values.
* No event occurs from this command
*
* @param settings A pointer to an iss360Settings_t struct in which the defualt values will be written
* @return
*		 - TRUE if succeeded
*		 - FALSE if parameter was NULLPTR
***************************************************************************************************/
bool_t islSdkIss360GetDefaultSettings(iss360Settings_t* settings);

/***************************************************************************************************
* Saves iss360 settings to file.
* No event occurs from this command
*
* @param info A pointer to deviceInfo_t struct
* @param settings A pointer to an iss360Settings_t struct containing the settings to save
* @param cal A pointer to an iss360AhrsCal_t struct containing the calibration to save
* @param fileName A pointer to a UTF8 string holding the file path and name
* @note This generates an xml file. The \p fileName should contain the extension .xml
* @return
*		 - TRUE if succeeded
*		 - FALSE if failed
***************************************************************************************************/
bool_t islSdkIss360SaveSettingsToFile(const deviceInfo_t* info, const iss360Settings_t* settings, const iss360AhrsCal_t* cal, const str_t* fileName);

/***************************************************************************************************
* Load iss360 settings from file.
* No event occurs from this command
*
* @param info A pointer to deviceInfo_t struct in which the file values will be written
* @param settings A pointer to an iss360Settings_t struct in which the file values will be written
* @param cal A pointer to an iss360AhrsCal_t struct in which the file values will be written
* @param fileName A pointer to a UTF8 string holding the file path and name to load
* @return
*		 - TRUE if succeeded
*		 - FALSE if failed
***************************************************************************************************/
bool_t islSdkIss360LoadSettingsFromFile(deviceInfo_t* info, iss360Settings_t* settings, iss360AhrsCal_t* cal, const str_t* fileName);

/***************************************************************************************************
* Get the raw signal buffer data.
* The full ADC return waveform and computed cross correlation waveform can be read using this function.
* The sample rate of the data is either 2.5Mhz or 5Mhz depending on the settings. This function is intended for test and R&D
* This will trigger an iss360 events with the {@link sdkEventHandler_t eventId} parameter set to ::EVENT_ISS360_BUF_DATA
*
* @param id The id of the iss360 as returned by the SDK
* @param bufId 0 = ADC buffer, 1 = cross correlation buffer
* @param index The offset in the buffer to read from.
* @param limit The maximum number of samples to read.
***************************************************************************************************/
void islSdkIss360ReadBuffer(uint32_t id, uint8_t bufId, uint32_t startSample, uint32_t maxSamples);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
