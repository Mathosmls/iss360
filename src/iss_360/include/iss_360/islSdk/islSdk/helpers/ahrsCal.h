#ifndef AHRSCAL_H_
#define AHRSCAL_H_
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* @file This a stand alone module for calculating the transformation matrix and offsets to remap data
* points that lie on the surface of an ellipsoid to lie on the surface of a sphere. As required by
* a mangnetometer calibration.
***************************************************************************************************/

//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include "islSdkVector.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

typedef struct accelCal_t accelCal_t;
typedef struct magCal_t magCal_t;

//---------------------------------- Public Function Prototypes -----------------------------------

accelCal_t* accelCalCreate(uint32_t samplesPerAverage, real_t maxAllowableG, real_t cardinalAxisDeviation);
void accelCalDelete(accelCal_t* inst);
void accelCalReset(accelCal_t* inst);
void accelCalStart(accelCal_t* inst);
bool_t accelCalGetData(accelCal_t* inst, uint32_t idx, vector3_t* v);
uint32_t accelCalAddData(accelCal_t* inst, const vector3_t* accel);

/***************************************************************************************************
* Calculates the transformation matrix and bias values for an accelerometer.
* @param inst A pointer to the object
* @param transform A pointer to a matrix3x3_t where the matrix will be written
* @param bias A pointer to a vector_t where the bias will be written
* @return True if success
***************************************************************************************************/
bool_t accelCalCalculateMatrix(accelCal_t* inst, vector3_t* bias, matrix3x3_t* transform);

/***************************************************************************************************
* Creates an object to manage the calibration of the magnetometer or accelerometer.
* @param sampleCount The number of sections the surface of the sphere should be split into.
* Each section holds 1 sensor vector reading. Limits are 0 to 32768
* @return A pointer to the object
***************************************************************************************************/
magCal_t* magCalCreate(uint16_t sampleCount);

/***************************************************************************************************
* Delete and free the memory for the created object.
* @param inst A pointer to the object
***************************************************************************************************/
void magCalDelete(magCal_t* inst);

/***************************************************************************************************
* Resets and clears the stored data
* @param inst A pointer to the object
***************************************************************************************************/
void magCalReset(magCal_t* inst);

/***************************************************************************************************
* Get the number of vectors stored by the object
* @param inst A pointer to the object
* @return The number of vectors stored by the object
***************************************************************************************************/
uint32_t magCalGetDataCount(const magCal_t* magCal);

/***************************************************************************************************
* Get the vectors stored by the object
* @param inst A pointer to the object
* @param list A Pointer to a vector array in which the stored vectors will be copied to
* @param count The number of elements in the passed vector array
* @param countOffset The offset in the objects vector array list from where to start copying from
* @param applyCal If true then use the calculated matrix and bias to calibrate the copied vectors
* @return The number of vectors copied
***************************************************************************************************/
uint32_t magCalGetData(const magCal_t* magCal, vector3_t* list, uint32_t count, uint32_t countOffset, bool_t applyCal);

/***************************************************************************************************
* Add a vector for use in the calculation of the matrix and bias
* @param inst A pointer to the object
* @param point A pointer to a vector of uncalibratied magnetometer data
* @param qualityFactorOut A pointer to a variable that will have the quality factor writen to it. 0 = poor, 1 = best
* @return The index of an array where the point was stored, -1 means is wasn't stored. This will typically increment
* to sampleCount on every call, except when data is updated.
***************************************************************************************************/
int32_t magCalAddData(magCal_t* magCal, const vector3_t* point, real_t* qualityFactorOut);

/***************************************************************************************************
* Calculate the correction matrix and bias
* This function first calculates a best fit ellipsoid for the list of vector points, built by magCalAddData().
* Then a correction matrix and offsets are calculated to transform the ellipsoid into a sphere centered on the x,y,z axis
* @param inst A pointer to the object
* @param radius the raduis of the sphere that points should fit to. Typically this would be the Earths magnetic field strength 54uT
* @param transform A pointer to a matrix3x3_t where the matrix will be written
* @param bias A pointer to a vector_t where the bias will be written
* @return 
*		 - TRUE if succeeded
*		 - FALSE \p transform and \p bias were not populated
***************************************************************************************************/
bool_t magCalCalculateMatrix(magCal_t* magCal, real_t radius, vector3_t* bias, matrix3x3_t* transform);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
