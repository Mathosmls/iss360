//------------------------------------------ Includes ----------------------------------------------

#include "ahrs.h"
#include "islSdkAhrs.h"
#include "platform/maths.h"
#include "platform/debug.h"

using namespace IslSdk;

//==================================================================================================
Ahrs::Ahrs(uint32_t deviceId) : m_deviceId(deviceId)
{
}
//--------------------------------------------------------------------------------------------------
Ahrs::~Ahrs()
{
}
//--------------------------------------------------------------------------------------------------
void Ahrs::setHeading(real_t headingRad)
{
	islSdkAhrsSetHeading(m_deviceId, headingRad);
}
//--------------------------------------------------------------------------------------------------
void Ahrs::setHeadingToMag()
{
	islSdkAhrsSetHeadingToMag(m_deviceId);
}
//--------------------------------------------------------------------------------------------------
void Ahrs::clearTurnsCount()
{
	islSdkAhrsClearTurnsCount(m_deviceId);
}
//==================================================================================================
GyroSensor::GyroSensor(uint32_t deviceId, uint32_t sensor) : m_deviceId(deviceId), m_sensor(sensor)
{
	vectorSet(&m_bias, 0);
}
//--------------------------------------------------------------------------------------------------
GyroSensor::~GyroSensor()
{
}
//--------------------------------------------------------------------------------------------------
void GyroSensor::updateCalValues(vector3_t& bias)
{
	if (!vectorEqual(&m_bias, &bias))
	{
		m_bias = bias;
		onCalChange(*this, m_bias);
	}
}
//--------------------------------------------------------------------------------------------------
void GyroSensor::autoCal()
{
	islSdkAhrsAutoGyroCal(m_deviceId, m_sensor);
	islSdkAhrsGetCal(m_deviceId);
}
//--------------------------------------------------------------------------------------------------
void GyroSensor::setCal(vector3_t& bias)
{
	updateCalValues(bias);
	islSdkAhrsSetGyroCal(m_deviceId, &bias, m_sensor);
}
//==================================================================================================
AccelSensor::AccelSensor(uint32_t deviceId, uint32_t sensor) : m_deviceId(deviceId), m_sensor(sensor)
{
	m_cal = nullptr;
	vectorSet(&m_bias, 0);
	matrixIdentity(&m_transform);
}
//--------------------------------------------------------------------------------------------------
AccelSensor::~AccelSensor()
{
	if (m_cal != nullptr)
	{
		accelCalDelete(m_cal);
	}
}
//--------------------------------------------------------------------------------------------------
void AccelSensor::updateCalValues(vector3_t& bias, matrix3x3_t& transform)
{
	if (!vectorEqual(&m_bias, &bias) || !matrixEqual(&m_transform, &transform))
	{
		m_bias = bias;
		m_transform = transform;
		onCalChange(*this, m_bias, m_transform);
	}
}
//--------------------------------------------------------------------------------------------------
void AccelSensor::newData(AccelSensor& accel, const vector3_t& v)
{
	if (m_cal != nullptr)
	{
		uint32_t progress = accelCalAddData(m_cal, &v);

		if (progress & 0xff00)
		{
			vector3_t avgVector;
			uint32_t idx = (progress >> 8) - 1;
			accelCalGetData(m_cal, idx, &avgVector);
			onCalProgress(*this, idx, avgVector, progress & 0xff);
		}
	}
}
//--------------------------------------------------------------------------------------------------
void AccelSensor::setCal(vector3_t& bias, matrix3x3_t& transform)
{
	islSdkAhrsSetAccelCal(m_deviceId, &bias, &transform, m_sensor);
	updateCalValues(bias, transform);
}
//--------------------------------------------------------------------------------------------------
void AccelSensor::startCal(uint32_t samplesPerAverage, real_t maxVariationG, real_t degFromCardinal)
{
	if (m_cal == nullptr)
	{
		m_cal = accelCalCreate(samplesPerAverage, maxVariationG, degToRad(degFromCardinal));
	}

	if (m_cal != nullptr)
	{
		onData.connect(slotData);
		accelCalStart(m_cal);
	}
}
//--------------------------------------------------------------------------------------------------
void AccelSensor::stopCal(bool_t cancel)
{
	matrix3x3_t transform;
	vector3_t bias;

	onData.disconnect(slotData);

	if (m_cal != nullptr)
	{
		if (!cancel && accelCalCalculateMatrix(m_cal, &bias, &transform))
		{
			vectorAdd(&bias, &bias, &m_bias);
			matrix3x3Multiply(&transform, &transform, &m_transform);
			setCal(bias, transform);
		}
		accelCalDelete(m_cal);
		m_cal = nullptr;
	}
}
//==================================================================================================
MagSensor::MagSensor(uint32_t deviceId, uint32_t sensor) : m_deviceId(deviceId), m_sensor(sensor)
{
	m_cal = nullptr;
	vectorSet(&m_bias, 0);
	matrixIdentity(&m_transform);
}
//--------------------------------------------------------------------------------------------------
MagSensor::~MagSensor()
{
	if (m_cal != nullptr)
	{
		magCalDelete(m_cal);
	}
}
//--------------------------------------------------------------------------------------------------
void MagSensor::updateCalValues(vector3_t& bias, matrix3x3_t& transform)
{
	if (!vectorEqual(&m_bias, &bias) || !matrixEqual(&m_transform, &transform))
	{
		m_bias = bias;
		m_transform = transform;
		onCalChange(*this, m_bias, m_transform);
	}
}
//--------------------------------------------------------------------------------------------------
void MagSensor::newData(MagSensor& mag, const vector3_t& v)
{
	if (m_cal != nullptr)
	{
		real_t qf;
		int32_t idx = magCalAddData(m_cal, &v, &qf);
		if (idx >= 0)
		{
			onCalProgress(*this, v, (uint32_t)idx, qf);
		}
	}
}
//--------------------------------------------------------------------------------------------------
void MagSensor::setCal(vector3_t& bias, matrix3x3_t& transform)
{
	islSdkAhrsSetMagCal(m_deviceId, &bias, &transform, m_sensor);
	updateCalValues(bias, transform);
}
//--------------------------------------------------------------------------------------------------
void MagSensor::startCal()
{
	if (m_cal == nullptr)
	{
		m_cal = magCalCreate(400);
	}

	if (m_cal != nullptr)
	{
		magCalReset(m_cal);
		onData.connect(slotData);
	}
}
//--------------------------------------------------------------------------------------------------
bool_t MagSensor::stopCal(bool_t cancel)
{
	matrix3x3_t transform;
	vector3_t bias;
	uint32_t count;
	bool_t ok = false;

	if (m_cal != nullptr)
	{
		if (!cancel)
		{
			count = magCalGetDataCount(m_cal);
			if (count > 10)
			{
				magCalCalculateMatrix(m_cal, 54.0, &bias, &transform);
				vectorAdd(&bias, &bias, &m_bias);
				matrix3x3Multiply(&transform, &transform, &m_transform);
				setCal(bias, transform);
				ok = true;
			}
		}
		magCalDelete(m_cal);
		m_cal = nullptr;
		onData.disconnect(slotData);
	}
	return ok;
}
//--------------------------------------------------------------------------------------------------
