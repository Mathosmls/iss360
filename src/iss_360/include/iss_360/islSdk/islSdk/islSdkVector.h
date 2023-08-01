#ifndef ISLSDKVECTOR_H_
#define ISLSDKVECTOR_H_
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"

//--------------------------------------- Public Constants -----------------------------------------

//----------------------------------------- Public Types ------------------------------------------

typedef struct
{
	real_t x;
	real_t y;
	real_t z;
} vector3_t;

typedef struct
{
	real_t w;
	real_t x;
	real_t y;
	real_t z;
} quaternion_t;

typedef struct
{
	real_t m[3][3];
} matrix3x3_t;

typedef struct
{
	real_t heading;
	real_t pitch;
	real_t roll;
} eulerAngles_t;

typedef enum
{
	VECTOR_AXIS_X_PLUS,
	VECTOR_AXIS_X_MINUS,
	VECTOR_AXIS_Y_PLUS,
	VECTOR_AXIS_Y_MINUS,
	VECTOR_AXIS_Z_PLUS,
	VECTOR_AXIS_Z_MINUS,
} vectorAxis_t;

//---------------------------------- Public Function Prototypes -----------------------------------

/***************************************************************************************************
* Test if 2 vectors are equal.
* @param a Pointer to a vector
* @param b Pointer to a vector
* @return True if equal
***************************************************************************************************/
bool_t vectorEqual(const vector3_t* a, const vector3_t* b);

/***************************************************************************************************
* Set the elements of a vector.
* @param a Pointer to vector to write to
* @param val Value to set all 3 vector elements to
***************************************************************************************************/
void vectorSet(vector3_t* a, real_t val);

/***************************************************************************************************
* Add 2 vectors.
* dst = a + b
* @param dst Pointer to a vector that will contain the result
* @param a Pointer to a vector
* @param b Pointer to a vector
***************************************************************************************************/
void vectorAdd(vector3_t* dst, const vector3_t* a, const vector3_t* b);

/***************************************************************************************************
* Subtract 2 vectors.
* dst = a - b
* @param dst Pointer to a vector that will contain the result
* @param a Pointer to a vector
* @param b Pointer to a vector
***************************************************************************************************/
void vectorSubtract(vector3_t* dst, const vector3_t* a, const vector3_t* b);

/***************************************************************************************************
* Multiply a vector.
* @param dst Pointer to a vector that will contain the result
* @param a Pointer to a vector
* @param m Value to multiply all 3 vector elements by
***************************************************************************************************/
void vectorMultiplyScalar(vector3_t* dst, const vector3_t* a, real_t m);

/***************************************************************************************************
* Dot product of 2 vectors.
* ||a|| * ||b|| * cos(angle between)
* @param a Pointer to a vector
* @param b Pointer to a vector
* @return a dot b
***************************************************************************************************/
real_t vectorDotProduct(const vector3_t* a, const vector3_t* b);

/***************************************************************************************************
* Cross product of 2 vectors.
* ||dst|| = ||a|| * ||b|| * sin(angle between)
* @param dst Pointer to a vector that will contain the result
* @param a Pointer to a vector
* @param b Pointer to a vector
***************************************************************************************************/
void vectorCrossProduct(vector3_t* dst, const vector3_t* a, const vector3_t* b);

/***************************************************************************************************
* Normalise a vector.
* @param a Pointer to a vector that will be normalised
* @return
*		 - TRUE if succeeded
*		 - FALSE if magnitude of \p a is zero
***************************************************************************************************/
bool_t vectorNormalise(vector3_t* a);

/***************************************************************************************************
* Magnitude of a vector.
* m = ||a||
* @param a Pointer to a vector
* @return Magnitude of the vector
***************************************************************************************************/
real_t vectorMagnitude(const vector3_t* a);

/***************************************************************************************************
* Find the closes cardinal axis.
* @param v Pointer to a vector
* @param cardinal Pointer to a vector that will contain the result
* @return the closes cardinal axis
***************************************************************************************************/
vectorAxis_t vectorFindCardinalAxis(const vector3_t* v, vector3_t* cardinal);

/***************************************************************************************************
* Add 2 quaternions.
* dst = a + b
* @param dst Pointer to a quaternion that will contain the result
* @param a Pointer to a quaternion
* @param b Pointer to a quaternion
***************************************************************************************************/
void quaternionAdd(quaternion_t* dst, const quaternion_t* a, const quaternion_t* b);

/***************************************************************************************************
* Multiply 2 quaternions.
* dst = a * b
* @param dst Pointer to a quaternion that will contain the result
* @param a Pointer to a quaternion
* @param b Pointer to a quaternion
***************************************************************************************************/
void quaternionMultiply(quaternion_t* dst, const quaternion_t* a, const quaternion_t* b);

/***************************************************************************************************
* Conjugate a quaternion.
* @param q Pointer to a quaternion that will be conjugated
***************************************************************************************************/
void quaternionConjugate(quaternion_t* q);

/***************************************************************************************************
* Normalise a quaternion.
* @param q Pointer to a quaternion that will be normalised
* @return
*		 - TRUE if succeeded
*		 - FALSE if magnitude of \p q is zero
***************************************************************************************************/
bool_t quaternionNormalise(quaternion_t* q);

/***************************************************************************************************
* Rotation a quaternion by a rotaion vector
* @param dst Pointer to a quaternion that will contain the result
* @param q Pointer to quaternion to be rotated
* @param v Pointer to the rotation vector
***************************************************************************************************/
void quaternionRotateByVector(quaternion_t* dst, const quaternion_t* q, const vector3_t* v);

/***************************************************************************************************
* Rotation a vector using a quaternion
* @param q Pointer to a quaternion
* @param v Pointer to vector to be rotated
* @param dst Pointer to the rotated vector
***************************************************************************************************/
void quaternionRotateVector(const quaternion_t* q, const vector3_t* v, vector3_t* dst);

/***************************************************************************************************
* Get a quaternion from a rotaion vector
* @param q Pointer to a quaternion that will contain the result
* @param r Pointer to the rotation vector
***************************************************************************************************/
void quaternionFromRotationVector(quaternion_t* q, const vector3_t* r);

/***************************************************************************************************
* Convert a quaternion to an axis and angle.
* @param q Pointer to a quaternion
* @param axis Pointer to a vector that will contain the result
* @return the angle in radians
***************************************************************************************************/
real_t quaternionToAxisAngle(const quaternion_t* q, vector3_t* axis);

/***************************************************************************************************
* Convert an axis and angle to a quaternion.
* @param q Pointer to a quaternion that will contain the result
* @param axis Pointer to a vector
* @param angle Angle in radians
***************************************************************************************************/
void quaternionFromAxisAngle(quaternion_t* q, const vector3_t* axis, real_t angle);

/***************************************************************************************************
* Convert a quaternion to a rotation matrix.
* @param q Pointer to a quaternion
* @param rm Pointer to a matrix where the result is written
***************************************************************************************************/
void quaternionToRotationMatrix(const quaternion_t* q, matrix3x3_t* rm);

/***************************************************************************************************
* Convert a rotation matrix to a quaternion.
* @param q Pointer to a quaternion where the result is written
* @param rm Pointer to a matrix
***************************************************************************************************/
void quaternionFromRotationMatrix(quaternion_t* q, const matrix3x3_t* rm);

/***************************************************************************************************
* Get the rotational difference between 2 quaternions about an axis 
* @param qNow Pointer to first quaternion
* @param qLast Pointer to second quaternion
* @param about Pointer to the axis about which to measure rotation
* @param earthFrame If true then the about axis is in the world frame. False is sensor frame.
* @return the angle in radians
***************************************************************************************************/
real_t quaternionAngleBetween(const quaternion_t* qNow, const quaternion_t* qLast, const vector3_t* about, bool_t earthFrame);

/***************************************************************************************************
* Get the rotation of a quaternion about an axis
* @param q Pointer to quaternion
* @param about Pointer to the axis about which to measure rotation
* @return the angle in radians
***************************************************************************************************/
real_t quaternionGetRotationAbout(const quaternion_t* q, const vector3_t* about);

/***************************************************************************************************
* Get the closest cardinal down axis and Euler offset angles from a quaternion.
* @param q Pointer to a quaternion
* @param down Pointer to a vector where the result is written
* @param offsets Pointer to struct where the heading, pitch and roll offsets from down are written
***************************************************************************************************/
void quaternionGetDownAndEulerOffsets(const quaternion_t* q, vector3_t* down, eulerAngles_t* offsets);

/***************************************************************************************************
* Get Euler angles from a quaternion.
* @param q Pointer to a quaternion
* @param headingOffset offset in radians to add to the heading
* @param dst Pointer to struct where the heading, pitch and roll are written
***************************************************************************************************/
void quaternionToEulerAngles(const quaternion_t* q, real_t headingOffset, eulerAngles_t* dst);

/***************************************************************************************************
* Get an identity matrix.
* @param mat Pointer to a matrix where the result is written
***************************************************************************************************/
void matrixIdentity(matrix3x3_t* mat);

/***************************************************************************************************
* Test if 2 matrices are equal.
* @param a Pointer to a matrix
* @param b Pointer to a matrix
* @return True if equal
***************************************************************************************************/
bool_t matrixEqual(const matrix3x3_t* a, const matrix3x3_t* b);

/***************************************************************************************************
* Construct a rotation matrix from a down and forward vector
* @param rm Pointer to a matrix where the result is written
* @param down Pointer to down vector
* @param forward Pointer to forward vector
***************************************************************************************************/
void matrixFromDownAndForward(matrix3x3_t* rm, const vector3_t* down, const vector3_t* forward);

/***************************************************************************************************
* Rotation a vector using a matrix
* @param rm Pointer to a rotation matrix
* @param v Pointer to vector to be rotated
* @param dst Pointer to the rotated vector
* @param transpose if true the transpose of the matrix is used
***************************************************************************************************/
void matrixRotateVector(const matrix3x3_t* rm, const vector3_t* v, vector3_t* dst, bool_t transpose);

/***************************************************************************************************
* multiply two Matrices together
* @param dst Pointer to a matrix where the result is written
* @param a Pointer to first matrix
* @param b Pointer to second matrix
***************************************************************************************************/
void matrix3x3Multiply(matrix3x3_t* dst, matrix3x3_t* a, matrix3x3_t* b);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
