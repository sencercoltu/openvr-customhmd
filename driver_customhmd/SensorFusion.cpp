#include "SensorFusion.h"



CSensorFusion::CSensorFusion()
{
	m_RotQuat = HmdQuaternion_Init(1, 0, 0, 0);
	ZeroMemory(eInt, sizeof(eInt));
	b_x = 1, b_z = 0; // reference direction of flux in earth frame
	w_bx = 0, w_by = 0, w_bz = 0; // estimate gyroscope biases error

}

CSensorFusion::~CSensorFusion()
{
}

vr::HmdQuaternion_t CSensorFusion::Fuse(SensorData *pData)
{	
	MadgwickQuaternionUpdate(
		pData->TimeElapsed, 

		pData->Accel[0],
		pData->Accel[1],
		pData->Accel[2],
		
		pData->Gyro[0],
		pData->Gyro[1],
		pData->Gyro[2],
		
		pData->Mag[0],
		pData->Mag[1],
		pData->Mag[2]
	);

	vr::HmdQuaternion_t ret;
	ret.w = m_RotQuat.w;
	ret.x = -m_RotQuat.y;
	ret.y = m_RotQuat.z;
	ret.z = -m_RotQuat.x;
	return ret;
}

vr::HmdQuaternion_t CSensorFusion::HmdQuaternion_Init(double w, double x, double y, double z)
{
	vr::HmdQuaternion_t quat;
	quat.w = w;
	quat.x = x;
	quat.y = y;
	quat.z = z;
	return quat;
}

void CSensorFusion::HmdMatrix_SetIdentity(vr::HmdMatrix34_t *pMatrix)
{
	pMatrix->m[0][0] = 1.f;
	pMatrix->m[0][1] = 0.f;
	pMatrix->m[0][2] = 0.f;
	pMatrix->m[0][3] = 0.f;
	pMatrix->m[1][0] = 0.f;
	pMatrix->m[1][1] = 1.f;
	pMatrix->m[1][2] = 0.f;
	pMatrix->m[1][3] = 0.f;
	pMatrix->m[2][0] = 0.f;
	pMatrix->m[2][1] = 0.f;
	pMatrix->m[2][2] = 1.f;
	pMatrix->m[2][3] = 0.f;
}

vr::HmdQuaternion_t CSensorFusion::QuatMultiply(const vr::HmdQuaternion_t *q1, const vr::HmdQuaternion_t *q2)
{
	vr::HmdQuaternion_t result;
	result.x = q1->w*q2->x + q1->x*q2->w + q1->y*q2->z - q1->z*q2->y;
	result.y = q1->w*q2->y - q1->x*q2->z + q1->y*q2->w + q1->z*q2->x;
	result.z = q1->w*q2->z + q1->x*q2->y - q1->y*q2->x + q1->z*q2->w;
	result.w = q1->w*q2->w - q1->x*q2->x - q1->y*q2->y - q1->z*q2->z;
	return result;
}

vr::HmdQuaternion_t CSensorFusion::QuatNormalize(const vr::HmdQuaternion_t *q)
{
	vr::HmdQuaternion_t result;
	double n = sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
	result.x = q->x / n;
	result.y = q->y / n;
	result.z = q->z / n;
	result.w = q->w / n;
	return result;
}

vr::HmdQuaternion_t CSensorFusion::QuatConjugate(const vr::HmdQuaternion_t *q)
{
	vr::HmdQuaternion_t result;
	result.x = -q->x;
	result.y = -q->y;
	result.z = -q->z;
	result.w = q->w;
	return result;
}

// Implementation of Sebastian Madgwick's "...efficient orientation filter for... inertial/magnetic sensor arrays"
// (see http://www.x-io.co.uk/category/open-source/ for examples and more details)
// which fuses acceleration, rotation rate, and magnetic moments to produce a quaternion-based estimate of absolute
// device orientation -- which can be converted to yaw, pitch, and roll. Useful for stabilizing quadcopters, etc.
// The performance of the orientation filter is at least as good as conventional Kalman-based filtering algorithms
// but is much less computationally intensive---it can be performed on a 3.3 V Pro Mini operating at 8 MHz!
void CSensorFusion::MadgwickQuaternionUpdate(double deltat, double ax, double ay, double az, double gx, double gy, double gz, double mx, double my, double mz)
{
	double q1 = m_RotQuat.w, q2 = m_RotQuat.x, q3 = m_RotQuat.y, q4 = m_RotQuat.z;   // short name local variable for readability
	double norm;
	double hx, hy, _2bx, _2bz;
	double s1, s2, s3, s4;
	double qDot1, qDot2, qDot3, qDot4;

	// Auxiliary variables to avoid repeated arithmetic
	double _2q1mx;
	double _2q1my;
	double _2q1mz;
	double _2q2mx;
	double _4bx;
	double _4bz;
	double _2q1 = 2.0 * q1;
	double _2q2 = 2.0 * q2;
	double _2q3 = 2.0 * q3;
	double _2q4 = 2.0 * q4;
	double _2q1q3 = 2.0 * q1 * q3;
	double _2q3q4 = 2.0 * q3 * q4;
	double q1q1 = q1 * q1;
	double q1q2 = q1 * q2;
	double q1q3 = q1 * q3;
	double q1q4 = q1 * q4;
	double q2q2 = q2 * q2;
	double q2q3 = q2 * q3;
	double q2q4 = q2 * q4;
	double q3q3 = q3 * q3;
	double q3q4 = q3 * q4;
	double q4q4 = q4 * q4;

	// Normalise accelerometer measurement
	norm = sqrt(ax * ax + ay * ay + az * az);
	if (norm == 0.0) return; // handle NaN
	norm = 1.0 / norm;
	ax *= norm;
	ay *= norm;
	az *= norm;

	// Normalise magnetometer measurement
	norm = sqrt(mx * mx + my * my + mz * mz);
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
	_2bx = sqrt(hx * hx + hy * hy);
	_2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
	_4bx = 2.0 * _2bx;
	_4bz = 2.0 * _2bz;

	// Gradient decent algorithm corrective step
	s1 = -_2q3 * (2.0 * q2q4 - _2q1q3 - ax) + _2q2 * (2.0 * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
	s2 = _2q4 * (2.0 * q2q4 - _2q1q3 - ax) + _2q1 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q2 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + _2bz * q4 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
	s3 = -_2q1 * (2.0 * q2q4 - _2q1q3 - ax) + _2q4 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q3 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
	s4 = _2q2 * (2.0 * q2q4 - _2q1q3 - ax) + _2q3 * (2.0 * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
	norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
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
	norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
	norm = 1.0 / norm;

	m_RotQuat.w = q1 * norm;
	m_RotQuat.x = q2 * norm;
	m_RotQuat.y = q3 * norm;
	m_RotQuat.z = q4 * norm;
}

// Similar to Madgwick scheme but uses proportional and integral filtering on the error between estimated reference vectors and
// measured ones. 
void CSensorFusion::MahonyQuaternionUpdate(double deltat, double ax, double ay, double az, double gx, double gy, double gz, double mx, double my, double mz)
{
	double q1 = m_RotQuat.w, q2 = m_RotQuat.x, q3 = m_RotQuat.y, q4 = m_RotQuat.z;   // short name local variable for readability
	double norm;
	double hx, hy, bx, bz;
	double vx, vy, vz, wx, wy, wz;
	double ex, ey, ez;
	double pa, pb, pc;

	// Auxiliary variables to avoid repeated arithmetic
	double q1q1 = q1 * q1;
	double q1q2 = q1 * q2;
	double q1q3 = q1 * q3;
	double q1q4 = q1 * q4;
	double q2q2 = q2 * q2;
	double q2q3 = q2 * q3;
	double q2q4 = q2 * q4;
	double q3q3 = q3 * q3;
	double q3q4 = q3 * q4;
	double q4q4 = q4 * q4;

	// Normalise accelerometer measurement
	norm = sqrt(ax * ax + ay * ay + az * az);
	if (norm == 0.0) return; // handle NaN
	norm = 1.0 / norm;        // use reciprocal or division
	ax *= norm;
	ay *= norm;
	az *= norm;

	// Normalise magnetometer measurement
	norm = sqrt(mx * mx + my * my + mz * mz);
	if (norm == 0.0) return; // handle NaN
	norm = 1.0 / norm;        // use reciprocal or division
	mx *= norm;
	my *= norm;
	mz *= norm;

	// Reerence direction o Earth's magnetic ield
	hx = 2.0 * mx * (0.5 - q3q3 - q4q4) + 2.0 * my * (q2q3 - q1q4) + 2.0 * mz * (q2q4 + q1q3);
	hy = 2.0 * mx * (q2q3 + q1q4) + 2.0 * my * (0.5 - q2q2 - q4q4) + 2.0 * mz * (q3q4 - q1q2);
	bx = sqrt((hx * hx) + (hy * hy));
	bz = 2.0 * mx * (q2q4 - q1q3) + 2.0 * my * (q3q4 + q1q2) + 2.0 * mz * (0.5 - q2q2 - q3q3);

	// Estimated direction o gravity and magnetic ield
	vx = 2.0 * (q2q4 - q1q3);
	vy = 2.0 * (q1q2 + q3q4);
	vz = q1q1 - q2q2 - q3q3 + q4q4;
	wx = 2.0 * bx * (0.5 - q3q3 - q4q4) + 2.0 * bz * (q2q4 - q1q3);
	wy = 2.0 * bx * (q2q3 - q1q4) + 2.0 * bz * (q1q2 + q3q4);
	wz = 2.0 * bx * (q1q3 + q2q4) + 2.0 * bz * (0.5 - q2q2 - q3q3);

	// Error is cross product between estimated direction and measured direction o gravity
	ex = (ay * vz - az * vy) + (my * wz - mz * wy);
	ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
	ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
	if (SF_Ki > 0.0)
	{
		eInt[0] += ex;      // accumulate integral error
		eInt[1] += ey;
		eInt[2] += ez;
	}
	else
	{
		eInt[0] = 0.0;     // prevent integral wind up
		eInt[1] = 0.0;
		eInt[2] = 0.0;
	}

	// Apply eedback terms
	gx = gx + SF_Kp * ex + SF_Ki * eInt[0];
	gy = gy + SF_Kp * ey + SF_Ki * eInt[1];
	gz = gz + SF_Kp * ez + SF_Ki * eInt[2];

	// Integrate rate o change o quaternion
	pa = q2;
	pb = q3;
	pc = q4;
	q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5 * deltat);
	q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5 * deltat);
	q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5 * deltat);
	q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5 * deltat);

	// Normalise quaternion
	norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
	norm = 1.0 / norm;
	m_RotQuat.w = q1 * norm;
	m_RotQuat.x = q2 * norm;
	m_RotQuat.y = q3 * norm;
	m_RotQuat.z = q4 * norm;
}
