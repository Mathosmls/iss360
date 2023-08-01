#ifndef GPSNMEAPARSER_H_
#define GPSNMEAPARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

/// Supported GPS string types
typedef enum
{
	GPS_STR_UNSUPPORTED,
	GPS_STR_GLL,
	GPS_STR_GGA,
	GPS_STR_GSV,
	GPS_STR_GSA,
	GPS_STR_VTG,
	GPS_STR_RMC,
} gpsStringType_t;

/// Struct populated from GPGLL string
typedef struct
{
	real_t latitudeDeg;			///< Latitude in degrees
	real_t longitudeDeg;		///< Longitude in degrees
	uint32_t hours;				///< UTC hours
	uint32_t minutes;			///< UTC minutes
	real_t seconds;				///< UTC seconds
	bool_t valid;				///< Data valid if true
	uint8_t mode;				///< 'A' = Autonomous, 'D' = Differential, 'E' = Estimated (dead reckoning), 'M' = Manual input, 'N' = Data not valid
} gpgll_t;

/// Struct populated from GPGGA string
typedef struct
{
	uint32_t hours;				///< UTC hours
	uint32_t minutes;			///< UTC minutes
	real_t seconds;				///< UTC seconds
	real_t latitudeDeg;			///< Latitude in degrees
	real_t longitudeDeg;		///< Longitude in degrees
	uint8_t quality;			///< 0 = no fix, 1-9 = fix
	uint32_t satellitesInUse;	///< Number of satellites in use
	real_t hdop;				///< Horizontal dilution of precision
	real_t altitude;			///< Antenna altitude above/below mean sea level in meters
	real_t undulation;			///< The relationship between the geoid and the WGS84 ellipsoid in meters
	uint32_t ageSeconds;		///< Age of correction data
	uint32_t stationId;			///< Differential base station ID
} gpgga_t;

typedef struct
{
	uint32_t prn;				///< Satellite PRN number
	int32_t elevationDeg;		///< Elevation, degrees, 90 maximum
	uint32_t azimuthDeg;		///< Azimuth, degrees True, 000 to 359
	uint32_t snr;				///< SNR 00-99 dB
} gpgsvSatellite_t;

/// Struct populated from GPGSV string
typedef struct
{
	uint32_t totalMessageCount;	///< Total number of messages
	uint32_t messageNumber;		///< Message number of this message
	uint32_t satellitesInView;	///< Total number of satellites in view
	gpgsvSatellite_t satellite[4];
} gpgsv_t;

/// Struct populated from GPGSA string
typedef struct
{
	uint8_t mode;				///< A = automatic, M = manual
	uint8_t fixType;			///< 1 = Fix not available, 2 = 2D, 3 = 3D
	uint32_t prn[12];			///< PRN number of the satellite, GPS = 1 to 32, SBAS = 33 to 64 (add 87 for PRN number), GLO = 65 to 96
	real_t pdop;				///< Position dilution of precision
	real_t hdop;				///< Horizontal dilution of precision
	real_t vdop;				///< Vertical dilution of precision
} gpgsa_t;

/// Struct populated from GPVTG string
typedef struct
{
	real_t headingTrue;			///< Heading in degrees from true North
	real_t headingMag;			///< Heading in degrees from magnetic North
	real_t speedKn;				///< Speed in Knots
	real_t speedKm;				///< Speed in km/hr
	uint8_t mode;				///< 'A' = Autonomous, 'D' = Differential, 'E' = Estimated (dead reckoning), 'M' = Manual input, 'N' = Data not valid
} gpvtg_t;

/// Struct populated from GPRMC string
typedef struct
{
	uint32_t hours;				///< UTC hours
	uint32_t minutes;			///< UTC minutes
	real_t seconds;				///< UTC seconds
	bool_t valid;				///< Data valid if true
	real_t latitudeDeg;			///< Latitude in degrees
	real_t longitudeDeg;		///< Longitude in degrees
	real_t speedKn;				///< Speed in Knots
	real_t headingTrue;			///< Heading in degrees from true North
	uint32_t date;				///< Date
	uint32_t month;				///< Month 
	uint32_t year;				///< Year
	real_t magVar;				///< Magnetic variation in degrees. Add to headingTrue to get mag heading. Negative values are East
	uint8_t mode;				///< 'A' = Autonomous, 'D' = Differential, 'E' = Estimated (dead reckoning), 'M' = Manual input, 'N' = Data not valid
} gprmc_t;

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Get the type of NMEA GPS string.
* @param str A pointer to the start of a NMEA GPS string. With or without the '$' is fine
* @return the type of string
***************************************************************************************************/
gpsStringType_t gpsGetStringType(const str_t* str);

/***************************************************************************************************
* Parse a NMEA GPS GPGLL string.
* @param str A pointer to the start of a NMEA GPS string. With or without the '$' is fine
* @param gpgll A pointer to a gpgll_t struct in which to store the parser values. All elements of the struct are set to 0 before parsing
* @return TRUE if there are no errors in the string data
***************************************************************************************************/
bool_t gpsParseStringGPGLL(const str_t* str, gpgll_t* gpgll);

/***************************************************************************************************
* Parse a NMEA GPS GPGGA string.
* @param str A pointer to the start of a NMEA GPS string. With or without the '$' is fine
* @param gpgga A pointer to a gpgga_t struct in which to store the parser values. All elements of the struct are set to 0 before parsing
* @return TRUE if there are no errors in the string data
***************************************************************************************************/
bool_t gpsParseStringGPGGA(const str_t* str, gpgga_t* gpgga);

/***************************************************************************************************
* Parse a NMEA GPS GPGSV string.
* @param str A pointer to the start of a NMEA GPS string. With or without the '$' is fine
* @param gpgsv A pointer to a gpgsv_t struct in which to store the parser values. All elements of the struct are set to 0 before parsing
* @return TRUE if there are no errors in the string data
***************************************************************************************************/
bool_t gpsParseStringGPGSV(const str_t* str, gpgsv_t* gpgsv);

/***************************************************************************************************
* Parse a NMEA GPS GPGSA string.
* @param str A pointer to the start of a NMEA GPS string. With or without the '$' is fine
* @param gpgsa A pointer to a gpgsa_t struct in which to store the parser values. All elements of the struct are set to 0 before parsing
* @return TRUE if there are no errors in the string data
***************************************************************************************************/
bool_t gpsParseStringGPGSA(const str_t* str, gpgsa_t* gpgsa);

/***************************************************************************************************
* Parse a NMEA GPS GPVTG string.
* @param str A pointer to the start of a NMEA GPS string. With or without the '$' is fine
* @param gpvtg A pointer to a gpvtg_t struct in which to store the parser values. All elements of the struct are set to 0 before parsing
* @return TRUE if there are no errors in the string data
***************************************************************************************************/
bool_t gpsParseStringGPVTG(const str_t* str, gpvtg_t* gpvtg);

/***************************************************************************************************
* Parse a NMEA GPS GPRMC string.
* @param str A pointer to the start of a NMEA GPS string. With or without the '$' is fine
* @param gprmc A pointer to a gprmc_t struct in which to store the parser values. All elements of the struct are set to 0 before parsing
* @return TRUE if there are no errors in the string data
***************************************************************************************************/
bool_t gpsParseStringGPRMC(const str_t* str, gprmc_t* gprmc);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
