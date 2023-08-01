#ifndef AHRS_H_
#define AHRS_H_

//------------------------------------------ Includes ----------------------------------------------

#include "device.h"
#include "sigSlot.h"
#include "islSdkVector.h"
#include "helpers/ahrsCal.h"

//--------------------------------------- Class Definition -----------------------------------------

namespace IslSdk
{
	class GyroSensor
	{
	private:
		uint32_t m_deviceId;
		uint32_t m_sensor;
		vector3_t m_bias;
		
	public:
		const uint32_t& deviceId = m_deviceId;
		const uint32_t& sensorNumber = m_sensor;
		const vector3_t& bias = m_bias;
		Signal<GyroSensor&, const vector3_t&> onData;
		Signal<GyroSensor&, const vector3_t&> onCalChange;
		
		GyroSensor(uint32_t deviceId, uint32_t sensor);
		~GyroSensor();
		void updateCalValues(vector3_t& bias);
		void autoCal();
		void setCal(vector3_t& bias);
	};

	class AccelSensor
	{
	private:
		uint32_t m_deviceId;
		uint32_t m_sensor;
		accelCal_t* m_cal;
		vector3_t m_bias;
		matrix3x3_t m_transform;
		void newData(AccelSensor&, const vector3_t& v);
		Slot<AccelSensor&, const vector3_t&> slotData{ this, &AccelSensor::newData };

	public:
		const uint32_t& deviceId = m_deviceId;
		const uint32_t& sensorNumber = m_sensor;
		const vector3_t& bias = m_bias;
		const matrix3x3_t& transform = m_transform;
		Signal<AccelSensor&,const vector3_t&> onData;
		Signal<AccelSensor&,const vector3_t&, const matrix3x3_t&> onCalChange;
		Signal<AccelSensor&,uint32_t, const vector3_t&, uint8_t> onCalProgress;

		AccelSensor(uint32_t deviceId, uint32_t sensor);
		~AccelSensor();
		void updateCalValues(vector3_t& bias, matrix3x3_t& transform);
		void setCal(vector3_t& bias, matrix3x3_t& transform);
		void startCal(uint32_t samplesPerAverage, real_t maxVariationG, real_t degFromCardinal);
		void stopCal(bool_t cancel);
	};

	class MagSensor
	{
	private:
		uint32_t m_deviceId;
		uint32_t m_sensor;
		magCal_t* m_cal;
		vector3_t m_bias;
		matrix3x3_t m_transform;
		void newData(MagSensor&, const vector3_t& v);
		Slot<MagSensor&, const vector3_t&> slotData{ this, &MagSensor::newData };
		
	public:
		const uint32_t& deviceId = m_deviceId;
		const uint32_t& sensorNumber = m_sensor;
		const vector3_t& bias = m_bias;
		const matrix3x3_t& transform = m_transform;
		Signal<MagSensor&, const vector3_t&> onData;
		Signal<MagSensor&, const vector3_t&, const matrix3x3_t&> onCalChange;
		Signal<MagSensor&, const vector3_t&, uint32_t, real_t> onCalProgress;

		MagSensor(uint32_t deviceId, uint32_t sensor);
		~MagSensor();
		void updateCalValues(vector3_t& bias, matrix3x3_t& transform);
		void setCal(vector3_t& magBias, matrix3x3_t& transform);
		void startCal();
		bool_t stopCal(bool_t cancel);
	};

	class Ahrs
	{
	private:
		uint32_t m_deviceId;

	public:
		const uint32_t& deviceId = m_deviceId;
		Signal<Ahrs&, uint64_t, const quaternion_t&, real_t, real_t> onData;

		Ahrs(uint32_t deviceId);
		~Ahrs();
		void setHeading(real_t headingRad);
		void setHeadingToMag();
		void clearTurnsCount();
	};
}
//--------------------------------------------------------------------------------------------------
#endif
