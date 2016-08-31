#include "SensorFusion.h"

#define RAD(a) (a*M_PI/180.0)

CSensorFusion::CSensorFusion(float aR, float gR, float mR)
{
	m_aR = aR;
	m_mR = mR;
	m_gR = gR;
	m_RotQuat = Quaternion();
}

CSensorFusion::~CSensorFusion()
{
}

Quaternion CSensorFusion::Value()
{
	Quaternion ret;
	ret.w = m_RotQuat.w;
	ret.x = -m_RotQuat.y;
	ret.y = m_RotQuat.z;
	ret.z = -m_RotQuat.x;
	return ret;	
}

Quaternion CSensorFusion::Fuse(SensorData *pData)
{	
	MadgwickQuaternionUpdate(
	//FilterUpdate(
	//MahonyQuaternionUpdate(
		pData->TimeElapsed, 

		pData->Accel[0] * m_aR,
		pData->Accel[1] * m_aR,
		pData->Accel[2] * m_aR,
		
		RAD(pData->Gyro[0] * m_gR),
		RAD(pData->Gyro[1] * m_gR),
		RAD(pData->Gyro[2] * m_gR),
		
		pData->Mag[0] * m_mR,
		pData->Mag[1] * m_mR,
		pData->Mag[2] * m_mR
	);

	return Value();
}

// Implementation of Sebastian Madgwick's "...efficient orientation filter for... inertial/magnetic sensor arrays"
// (see http://www.x-io.co.uk/category/open-source/ for examples and more details)
// which fuses acceleration, rotation rate, and magnetic moments to produce a quaternion-based estimate of absolute
// device orientation -- which can be converted to yaw, pitch, and roll. Useful for stabilizing quadcopters, etc.
// The performance of the orientation filter is at least as good as conventional Kalman-based filtering algorithms
// but is much less computationally intensive---it can be performed on a 3.3 V Pro Mini operating at 8 MHz!
void CSensorFusion::MadgwickQuaternionUpdate(float deltat, float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz)
{
	
	float q1 = m_RotQuat.w, q2 = m_RotQuat.x, q3 = m_RotQuat.y, q4 = m_RotQuat.z;   // short name local variable for readability
	float norm;
	float hx, hy, _2bx, _2bz;
	float s1, s2, s3, s4;
	float qDot1, qDot2, qDot3, qDot4;

	// Auxiliary variables to avoid repeated arithmetic
	float _2q1mx;
	float _2q1my;
	float _2q1mz;
	float _2q2mx;
	float _4bx;
	float _4bz;
	float _2q1 = 2.0 * q1;
	float _2q2 = 2.0 * q2;
	float _2q3 = 2.0 * q3;
	float _2q4 = 2.0 * q4;
	float _2q1q3 = 2.0 * q1 * q3;
	float _2q3q4 = 2.0 * q3 * q4;
	float q1q1 = q1 * q1;
	float q1q2 = q1 * q2;
	float q1q3 = q1 * q3;
	float q1q4 = q1 * q4;
	float q2q2 = q2 * q2;
	float q2q3 = q2 * q3;
	float q2q4 = q2 * q4;
	float q3q3 = q3 * q3;
	float q3q4 = q3 * q4;
	float q4q4 = q4 * q4;

	// Normalise accelerometer measurement
	norm = sqrtf(ax * ax + ay * ay + az * az);
	if (norm == 0.0) return; // handle NaN
	norm = 1.0 / norm;
	ax *= norm;
	ay *= norm;
	az *= norm;

	// Normalise magnetometer measurement
	norm = sqrtf(mx * mx + my * my + mz * mz);
	if (norm == 0.0) return; // handle NaN
	norm = 1.0 / norm;
	mx *= norm;
	my *= norm;
	mz *= norm;

	// Reerence direction o Earth's magnetic ield
	_2q1mx = 2.0 * q1 * mx;
	_2q1my = 2.0 * q1 * my;
	_2q1mz = 2.0 * q1 * mz;
	_2q2mx = 2.0 * q2 * mx;
	hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
	hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
	_2bx = sqrtf(hx * hx + hy * hy);
	_2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
	_4bx = 2.0 * _2bx;
	_4bz = 2.0 * _2bz;

	// Gradient decent algorithm corrective step
	s1 = -_2q3 * (2.0 * q2q4 - _2q1q3 - ax) + _2q2 * (2.0 * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
	s2 = _2q4 * (2.0 * q2q4 - _2q1q3 - ax) + _2q1 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q2 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + _2bz * q4 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
	s3 = -_2q1 * (2.0 * q2q4 - _2q1q3 - ax) + _2q4 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q3 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
	s4 = _2q2 * (2.0 * q2q4 - _2q1q3 - ax) + _2q3 * (2.0 * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
	norm = sqrtf(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
	norm = 1.0 / norm;
	s1 *= norm;
	s2 *= norm;
	s3 *= norm;
	s4 *= norm;

	// Compute rate o change o quaternion
	qDot1 = 0.5 * (-q2 * gx - q3 * gy - q4 * gz) - SF_Beta * s1;
	qDot2 = 0.5 * (q1 * gx + q3 * gz - q4 * gy) - SF_Beta * s2;
	qDot3 = 0.5 * (q1 * gy - q2 * gz + q4 * gx) - SF_Beta * s3;
	qDot4 = 0.5 * (q1 * gz + q2 * gy - q3 * gx) - SF_Beta * s4;

	// Integrate to yield quaternion
	q1 += qDot1 * deltat;
	q2 += qDot2 * deltat;
	q3 += qDot3 * deltat;
	q4 += qDot4 * deltat;
	norm = sqrtf(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
	norm = 1.0 / norm;

	m_RotQuat.w = q1 * norm;
	m_RotQuat.x = q2 * norm;
	m_RotQuat.y = q3 * norm;
	m_RotQuat.z = q4 * norm;
}

void SmoothSensorData(float *newData, float *oldData, float mag)
{
	for (int i=0; i<3; i++)
	{
		float diff = abs(oldData[i]-newData[i]);
		float chg = diff / mag;
		if (chg > 0.2f)
			chg = 1.0f;
		oldData[i] = (newData[i] * chg) + (oldData[i] * (1.0f - chg));
	}
}