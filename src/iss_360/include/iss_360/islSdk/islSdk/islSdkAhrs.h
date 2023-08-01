#ifndef ISLSDKAHRS_H_
#define ISLSDKAHRS_H_
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "islSdkVector.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Get the gyro, accel and mag calibrations.
*
* @param id The id of the isa500 as returned by the SDK
***************************************************************************************************/
void islSdkAhrsGetCal(uint32_t id);

/***************************************************************************************************
* Set the gyro calibration.
* No event occurs from this command
*
* @param id The id of the isa500 as returned by the SDK
* @param bias The offsets to subtract from the gyro readings in degrees per second
***************************************************************************************************/
void islSdkAhrsSetGyroCal(uint32_t id, const vector3_t* bias, uint8_t sensor);

/***************************************************************************************************
* Set the accelerometer calibration.
* No event occurs from this command
*
* @param id The id of the isa500 as returned by the SDK
* @param bias The offsets to subtract from the accelerometer readings in G
* @param transform A matrix to translate accelerometers readings
***************************************************************************************************/
void islSdkAhrsSetAccelCal(uint32_t id, const vector3_t* bias, const matrix3x3_t* transform, uint8_t sensor);

/***************************************************************************************************
* Set the magnetometer calibration.
* No event occurs from this command
*
* @param id The id of the isa500 as returned by the SDK
* @param bias The offsets to subtract from the magnetometer in micro Tesla
* @param transform A matrix to translate magnetometer readings that lie on the surface of an
* elipsoid to lie on the surface of a sphere who's raduis equals the Earths magnetic field strength
***************************************************************************************************/
void islSdkAhrsSetMagCal(uint32_t id, const vector3_t* bias, const matrix3x3_t* transform, uint8_t sensor);

/***************************************************************************************************
* Automatically set the gyro calibration.
* No event occurs from this command
*
* @param id The id of the isa500 as returned by the SDK
* @warning The device must be absolutely stationary when this command is issued
***************************************************************************************************/
void islSdkAhrsAutoGyroCal(uint32_t id, uint8_t sensor);

/***************************************************************************************************
* Set the AHRS heading to a specified angle.
*
* @param id The id of the isa500 as returned by the SDK
* @param angle The angle in radians to set the heading to
***************************************************************************************************/
void islSdkAhrsSetHeading(uint32_t id, real_t angle);

/***************************************************************************************************
* Set the AHRS heading to the magnetometer heading.
*
* @param id The id of the isa500 as returned by the SDK
***************************************************************************************************/
void islSdkAhrsSetHeadingToMag(uint32_t id);

/***************************************************************************************************
* Reset the turns count to zero.
*
* @param id The id of the isa500 as returned by the SDK
***************************************************************************************************/
void islSdkAhrsClearTurnsCount(uint32_t id);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
