
#ifndef MATHS_H_
#define MATHS_H_
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------ Includes ----------------------------------------------

#include "platform/sdkTypes.h"
#include <math.h>
#include <stdlib.h>
#include <float.h>

//--------------------------------------- Public Constants -----------------------------------------

#define MATH_PI ((real_t)3.1415926535897932384626433832795)
#define MATH_2PI ((real_t)2.0 * MATH_PI)
#define MATH_ROOT_HALF ((real_t)0.70710678118654752440084436210485)

#define mathIsNanF(v) isnan(v)
#define mathIsInfF(v) isinf(v)
#define mathAbsF(v) fabsf(v)
#define mathModF(v1, v2) fmodf(v1, v2)
#define mathSinF(v) sinf(v)
#define mathCosF(v) cosf(v)
#define mathTanF(v) tanf(v)
#define mathAsinF(v) asinf(v)
#define mathAcosF(v) acosf(v)
#define mathAtanF(v) atanf(v)
#define mathAtan2F(y, x) atan2f(y, x)
#define mathLogF(v) logf(v)
#define mathLog10F(v) log10f(v)
#define mathSqrtF(v) sqrtf(v)
#define mathPowerF(v, power) powf(v, power)

#define mathIsNanD(v) isnan(v)
#define mathIsInfD(v) isinf(v)
#define mathAbsD(v) fabs(v)
#define mathModD(v, v2) fmod(v, v2)
#define mathSinD(v) sin(v)
#define mathCosD(v) cos(v)
#define mathTanD(v) tan(v)
#define mathAsinD(v) asin(v)
#define mathAcosD(v) acos(v)
#define mathAtanD(v) atan(v)
#define mathAtan2D(y, x) atan2(y, x)
#define mathLogD(v) log(v)
#define mathLog10D(v) log10(v)
#define mathSqrtD(v) sqrt(v)
#define mathPowerD(v, power) pow(v, power)

#if MATH_USE_DOUBLE != 0
#define mathIsNan(v) mathIsNanD(v)
#define mathIsInf(v) mathIsInfD(v)
#define mathAbs(v) mathAbsD(v)
#define mathMod(v1, v2) mathModD(v1, v2)
#define mathSin(v) mathSinD(v)
#define mathCos(v) mathCosD(v)
#define mathTan(v) mathTanD(v)
#define mathAsin(v) mathAsinD(v)
#define mathAcos(v) mathAcosD(v)
#define mathAtan(v) mathAtanD(v)
#define mathAtan2(y, x) mathAtan2D(y, x)
#define mathLog(v) mathLogD(v)
#define mathLog10(v) mathLog10D(v)
#define mathSqrt(v) mathSqrtD(v)
#define mathPower(v, power) mathPowerD(v, power)
#define REAL_MAX DBL_MAX
#define REAL_MIN -DBL_MAX
#else
#define mathIsNan(v) mathIsNanF(v)
#define mathIsInf(v) mathIsInfF(v)
#define mathAbs(v) mathAbsF(v)
#define mathMod(v1, v2) mathModF(v1, v2)
#define mathSin(v) mathSinF(v)
#define mathCos(v) mathCosF(v)
#define mathTan(v) mathTanF(v)
#define mathAsin(v) mathAsinF(v)
#define mathAcos(v) mathAcosF(v)
#define mathAtan(v) mathAtanF(v)
#define mathAtan2(y, x) mathAtan2F(y, x)
#define mathLog(v) mathLogF(v)
#define mathLog10(v) mathLog10F(v)
#define mathSqrt(v) mathSqrtF(v)
#define mathPower(v, power) mathPowerF(v, power)
#define REAL_MAX FLT_MAX
#define REAL_MIN -FLT_MAX
#endif

#define degToRad(deg) ((real_t)(deg) * (MATH_PI / (real_t)180.0))
#define radToDeg(rad) ((real_t)(rad) * ((real_t)180.0 / MATH_PI))
#define mathAbsInt(v) abs(v)
#define mathMax(a, b) ((a) >= (b) ? (a) : (b))
#define mathMin(a, b) ((a) <= (b) ? (a) : (b))
#define mathLimit(v, ll, ul) mathMin(mathMax(v, ll), ul)
#define mathCompare(a, b, epsilon) (mathAbs(a - b) < epsilon)

//----------------------------------------- Public Types ------------------------------------------

//---------------------------------- Public Function Prototypes -----------------------------------

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif
