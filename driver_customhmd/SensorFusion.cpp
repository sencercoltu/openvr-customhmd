#include "SensorFusion.h"



CSensorFusion::CSensorFusion()
{
	m_RotQuat = Quaternion();
	//ZeroMemory(eInt, sizeof(eInt));
	//b_x = 1, b_z = 0; // reference direction of flux in earth frame
	//w_bx = 0, w_by = 0, w_bz = 0; // estimate gyroscope biases error

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

	return Value();
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

//
//vr::HmdQuaternion_t CSensorFusion::QuatMultiply(const vr::HmdQuaternion_t *q1, const vr::HmdQuaternion_t *q2)
//{
//	vr::HmdQuaternion_t result;
//	result.x = q1->w*q2->x + q1->x*q2->w + q1->y*q2->z - q1->z*q2->y;
//	result.y = q1->w*q2->y - q1->x*q2->z + q1->y*q2->w + q1->z*q2->x;
//	result.z = q1->w*q2->z + q1->x*q2->y - q1->y*q2->x + q1->z*q2->w;
//	result.w = q1->w*q2->w - q1->x*q2->x - q1->y*q2->y - q1->z*q2->z;
//	return result;
//}
//
//vr::HmdQuaternion_t CSensorFusion::QuatNormalize(const vr::HmdQuaternion_t *q)
//{
//	vr::HmdQuaternion_t result;
//	double n = sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
//	result.x = q->x / n;
//	result.y = q->y / n;
//	result.z = q->z / n;
//	result.w = q->w / n;
//	return result;
//}
//
//vr::HmdQuaternion_t CSensorFusion::QuatConjugate(const vr::HmdQuaternion_t *q)
//{
//	vr::HmdQuaternion_t result;
//	result.x = -q->x;
//	result.y = -q->y;
//	result.z = -q->z;
//	result.w = q->w;
//	return result;
//}
//
//vr::HmdQuaternion_t CSensorFusion::QuatInverse(const vr::HmdQuaternion_t *q)
//{
//	double n = q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w;
//	vr::HmdQuaternion_t result;
//	result.x = -q->x / n;
//	result.y = -q->y / n;
//	result.z = -q->z / n;
//	result.w = q->w / n;
//	return result;
//}
//
//vr::HmdVector3d_t CSensorFusion::QuaterionToEuler(const vr::HmdQuaternion_t *q)
//{
//	vr::HmdVector3d_t e;
//
//	double test = q->x*q->y + q->z*q->w;
//	if (test > 0.499) { // singularity at north pole
//		e.v[0] = 2 * atan2(q->x, q->w);
//		e.v[1] = M_PI / 2.0;
//		e.v[2] = 0;
//		return e;
//	}
//	if (test < -0.499) { // singularity at south pole
//		e.v[0] = -2 * atan2(q->x, q->w);
//		e.v[1] = -M_PI / 2;
//		e.v[2] = 0;
//		return e;
//	}
//	double sqx = q->x*q->x;
//	double sqy = q->y*q->y;
//	double sqz = q->z*q->z;
//	e.v[0] = atan2(2 * q->y*q->w - 2 * q->x*q->z, 1 - 2 * sqy - 2 * sqz);
//	e.v[1] = asin(2 * test);
//	e.v[2] = atan2(2 * q->x*q->w - 2 * q->y*q->z, 1 - 2 * sqx - 2 * sqz);
//	return e;
//}
//
//vr::HmdQuaternion_t CSensorFusion::EulerToQuaternion(const vr::HmdVector3d_t *e)
//{
//	vr::HmdQuaternion_t q;
//	// Assuming the angles are in radians.
//	double c1 = cos(e->v[0] / 2);
//	double s1 = sin(e->v[0] / 2);
//	double c2 = cos(e->v[1] / 2);
//	double s2 = sin(e->v[1] / 2);
//	double c3 = cos(e->v[2] / 2);
//	double s3 = sin(e->v[2] / 2);
//	double c1c2 = c1*c2;
//	double s1s2 = s1*s2;
//	q.w = c1c2*c3 - s1s2*s3;
//	q.x = c1c2*s3 + s1s2*c3;
//	q.y = s1*c2*c3 + c1*s2*s3;
//	q.z = c1*s2*c3 - s1*c2*s3;
//	return q;
//}


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
//void CSensorFusion::MahonyQuaternionUpdate(double deltat, double ax, double ay, double az, double gx, double gy, double gz, double mx, double my, double mz)
//{
//	double q1 = m_RotQuat.w, q2 = m_RotQuat.x, q3 = m_RotQuat.y, q4 = m_RotQuat.z;   // short name local variable for readability
//	double norm;
//	double hx, hy, bx, bz;
//	double vx, vy, vz, wx, wy, wz;
//	double ex, ey, ez;
//	double pa, pb, pc;
//
//	// Auxiliary variables to avoid repeated arithmetic
//	double q1q1 = q1 * q1;
//	double q1q2 = q1 * q2;
//	double q1q3 = q1 * q3;
//	double q1q4 = q1 * q4;
//	double q2q2 = q2 * q2;
//	double q2q3 = q2 * q3;
//	double q2q4 = q2 * q4;
//	double q3q3 = q3 * q3;
//	double q3q4 = q3 * q4;
//	double q4q4 = q4 * q4;
//
//	// Normalise accelerometer measurement
//	norm = sqrt(ax * ax + ay * ay + az * az);
//	if (norm == 0.0) return; // handle NaN
//	norm = 1.0 / norm;        // use reciprocal or division
//	ax *= norm;
//	ay *= norm;
//	az *= norm;
//
//	// Normalise magnetometer measurement
//	norm = sqrt(mx * mx + my * my + mz * mz);
//	if (norm == 0.0) return; // handle NaN
//	norm = 1.0 / norm;        // use reciprocal or division
//	mx *= norm;
//	my *= norm;
//	mz *= norm;
//
//	// Reerence direction o Earth's magnetic ield
//	hx = 2.0 * mx * (0.5 - q3q3 - q4q4) + 2.0 * my * (q2q3 - q1q4) + 2.0 * mz * (q2q4 + q1q3);
//	hy = 2.0 * mx * (q2q3 + q1q4) + 2.0 * my * (0.5 - q2q2 - q4q4) + 2.0 * mz * (q3q4 - q1q2);
//	bx = sqrt((hx * hx) + (hy * hy));
//	bz = 2.0 * mx * (q2q4 - q1q3) + 2.0 * my * (q3q4 + q1q2) + 2.0 * mz * (0.5 - q2q2 - q3q3);
//
//	// Estimated direction o gravity and magnetic ield
//	vx = 2.0 * (q2q4 - q1q3);
//	vy = 2.0 * (q1q2 + q3q4);
//	vz = q1q1 - q2q2 - q3q3 + q4q4;
//	wx = 2.0 * bx * (0.5 - q3q3 - q4q4) + 2.0 * bz * (q2q4 - q1q3);
//	wy = 2.0 * bx * (q2q3 - q1q4) + 2.0 * bz * (q1q2 + q3q4);
//	wz = 2.0 * bx * (q1q3 + q2q4) + 2.0 * bz * (0.5 - q2q2 - q3q3);
//
//	// Error is cross product between estimated direction and measured direction o gravity
//	ex = (ay * vz - az * vy) + (my * wz - mz * wy);
//	ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
//	ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
//	if (SF_Ki > 0.0)
//	{
//		eInt[0] += ex;      // accumulate integral error
//		eInt[1] += ey;
//		eInt[2] += ez;
//	}
//	else
//	{
//		eInt[0] = 0.0;     // prevent integral wind up
//		eInt[1] = 0.0;
//		eInt[2] = 0.0;
//	}
//
//	// Apply eedback terms
//	gx = gx + SF_Kp * ex + SF_Ki * eInt[0];
//	gy = gy + SF_Kp * ey + SF_Ki * eInt[1];
//	gz = gz + SF_Kp * ez + SF_Ki * eInt[2];
//
//	// Integrate rate o change o quaternion
//	pa = q2;
//	pb = q3;
//	pc = q4;
//	q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5 * deltat);
//	q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5 * deltat);
//	q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5 * deltat);
//	q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5 * deltat);
//
//	// Normalise quaternion
//	norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
//	norm = 1.0 / norm;
//	m_RotQuat.w = q1 * norm;
//	m_RotQuat.x = q2 * norm;
//	m_RotQuat.y = q3 * norm;
//	m_RotQuat.z = q4 * norm;
//}
//
//// Function to compute one filter iteration
//void CSensorFusion::FilterUpdate(double deltat, double a_x, double a_y, double a_z, double w_x, double w_y, double w_z, double m_x, double m_y, double m_z)
//{
//	// local system variables
//	double norm; // vector norm
//	double SEqDot_omega_1, SEqDot_omega_2, SEqDot_omega_3, SEqDot_omega_4; // quaternion rate from gyroscopes elements
//	double f_1, f_2, f_3, f_4, f_5, f_6; // objective function elements
//	double J_11or24, J_12or23, J_13or22, J_14or21, J_32, J_33, // objective function Jacobian elements
//		J_41, J_42, J_43, J_44, J_51, J_52, J_53, J_54, J_61, J_62, J_63, J_64; //
//	double SEqHatDot_1, SEqHatDot_2, SEqHatDot_3, SEqHatDot_4; // estimated direction of the gyroscope error
//	double w_err_x, w_err_y, w_err_z; // estimated direction of the gyroscope error (angular)
//	double h_x, h_y, h_z; // computed flux in the earth frame
//						 // axulirary variables to avoid reapeated calcualtions
//	double halfSEq_1 = 0.5f *  m_RotQuat.w;
//	double halfSEq_2 = 0.5f *  m_RotQuat.x;
//	double halfSEq_3 = 0.5f *  m_RotQuat.y;
//	double halfSEq_4 = 0.5f *  m_RotQuat.z;
//	double twoSEq_1 = 2.0f *  m_RotQuat.w;
//	double twoSEq_2 = 2.0f *  m_RotQuat.x;
//	double twoSEq_3 = 2.0f *  m_RotQuat.y;
//	double twoSEq_4 = 2.0f *  m_RotQuat.z;
//	double twob_x = 2.0f * b_x;
//	double twob_z = 2.0f * b_z;
//	double twob_xSEq_1 = 2.0f * b_x *  m_RotQuat.w;
//	double twob_xSEq_2 = 2.0f * b_x *  m_RotQuat.x;
//	double twob_xSEq_3 = 2.0f * b_x *  m_RotQuat.y;
//	double twob_xSEq_4 = 2.0f * b_x *  m_RotQuat.z;
//	double twob_zSEq_1 = 2.0f * b_z *  m_RotQuat.w;
//	double twob_zSEq_2 = 2.0f * b_z *  m_RotQuat.x;
//	double twob_zSEq_3 = 2.0f * b_z *  m_RotQuat.y;
//	double twob_zSEq_4 = 2.0f * b_z *  m_RotQuat.z;
//	double SEq_1SEq_2;
//	double SEq_1SEq_3 = m_RotQuat.w *  m_RotQuat.y;
//	double SEq_1SEq_4;
//	double SEq_2SEq_3;
//	double SEq_2SEq_4 = m_RotQuat.x *  m_RotQuat.z;
//	double SEq_3SEq_4;
//	double twom_x = 2.0f * m_x;
//	double twom_y = 2.0f * m_y;
//	double twom_z = 2.0f * m_z;
//	// normalise the accelerometer measurement
//	norm = sqrt(a_x * a_x + a_y * a_y + a_z * a_z);
//	a_x /= norm;
//	a_y /= norm;
//	a_z /= norm;
//	// normalise the magnetometer measurement
//	norm = sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
//	m_x /= norm;
//	m_y /= norm;
//	m_z /= norm;
//	// compute the objective function and Jacobian
//	f_1 = twoSEq_2 *  m_RotQuat.z - twoSEq_1 *  m_RotQuat.y - a_x;
//	f_2 = twoSEq_1 *  m_RotQuat.x + twoSEq_3 *  m_RotQuat.z - a_y;
//	f_3 = 1.0f - twoSEq_2 *  m_RotQuat.x - twoSEq_3 *  m_RotQuat.y - a_z;
//	f_4 = twob_x * (0.5f - m_RotQuat.y *  m_RotQuat.y - m_RotQuat.z *  m_RotQuat.z) + twob_z * (SEq_2SEq_4 - SEq_1SEq_3) - m_x;
//	f_5 = twob_x * (m_RotQuat.x *  m_RotQuat.y - m_RotQuat.w *  m_RotQuat.z) + twob_z * (m_RotQuat.w *  m_RotQuat.x + m_RotQuat.y *  m_RotQuat.z) - m_y;
//	f_6 = twob_x * (SEq_1SEq_3 + SEq_2SEq_4) + twob_z * (0.5f - m_RotQuat.x *  m_RotQuat.x - m_RotQuat.y *  m_RotQuat.y) - m_z;
//	J_11or24 = twoSEq_3; // J_11 negated in matrix multiplication
//	J_12or23 = 2.0f *  m_RotQuat.z;
//	J_13or22 = twoSEq_1; // J_12 negated in matrix multiplication
//	J_14or21 = twoSEq_2;
//	J_32 = 2.0f * J_14or21; // negated in matrix multiplication
//	J_33 = 2.0f * J_11or24; // negated in matrix multiplication
//	J_41 = twob_zSEq_3; // negated in matrix multiplication
//	J_42 = twob_zSEq_4;
//	J_43 = 2.0f * twob_xSEq_3 + twob_zSEq_1; // negated in matrix multiplication
//	J_44 = 2.0f * twob_xSEq_4 - twob_zSEq_2; // negated in matrix multiplication
//	J_51 = twob_xSEq_4 - twob_zSEq_2; // negated in matrix multiplication
//	J_52 = twob_xSEq_3 + twob_zSEq_1;
//	J_53 = twob_xSEq_2 + twob_zSEq_4;
//	J_54 = twob_xSEq_1 - twob_zSEq_3; // negated in matrix multiplication
//	J_61 = twob_xSEq_3;
//	J_62 = twob_xSEq_4 - 2.0f * twob_zSEq_2;
//	J_63 = twob_xSEq_1 - 2.0f * twob_zSEq_3;
//	J_64 = twob_xSEq_2;
//	// compute the gradient (matrix multiplication)
//	SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1 - J_41 * f_4 - J_51 * f_5 + J_61 * f_6;
//	SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32 * f_3 + J_42 * f_4 + J_52 * f_5 + J_62 * f_6;
//	SEqHatDot_3 = J_12or23 * f_2 - J_33 * f_3 - J_13or22 * f_1 - J_43 * f_4 + J_53 * f_5 + J_63 * f_6;
//	SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2 - J_44 * f_4 - J_54 * f_5 + J_64 * f_6;
//	// normalise the gradient to estimate direction of the gyroscope error
//	norm = sqrt(SEqHatDot_1 * SEqHatDot_1 + SEqHatDot_2 * SEqHatDot_2 + SEqHatDot_3 * SEqHatDot_3 + SEqHatDot_4 * SEqHatDot_4);
//	SEqHatDot_1 = SEqHatDot_1 / norm;
//	SEqHatDot_2 = SEqHatDot_2 / norm;
//	SEqHatDot_3 = SEqHatDot_3 / norm;
//	SEqHatDot_4 = SEqHatDot_4 / norm;
//	// compute angular estimated direction of the gyroscope error
//	w_err_x = twoSEq_1 * SEqHatDot_2 - twoSEq_2 * SEqHatDot_1 - twoSEq_3 * SEqHatDot_4 + twoSEq_4 * SEqHatDot_3;
//	w_err_y = twoSEq_1 * SEqHatDot_3 + twoSEq_2 * SEqHatDot_4 - twoSEq_3 * SEqHatDot_1 - twoSEq_4 * SEqHatDot_2;
//	w_err_z = twoSEq_1 * SEqHatDot_4 - twoSEq_2 * SEqHatDot_3 + twoSEq_3 * SEqHatDot_2 - twoSEq_4 * SEqHatDot_1;
//	// compute and remove the gyroscope baises
//	w_bx += w_err_x * deltat * SF_Zeta;
//	w_by += w_err_y * deltat * SF_Zeta;
//	w_bz += w_err_z * deltat * SF_Zeta;
//	w_x -= w_bx;
//	w_y -= w_by;
//	w_z -= w_bz;
//	// compute the quaternion rate measured by gyroscopes
//	SEqDot_omega_1 = -halfSEq_2 * w_x - halfSEq_3 * w_y - halfSEq_4 * w_z;
//	SEqDot_omega_2 = halfSEq_1 * w_x + halfSEq_3 * w_z - halfSEq_4 * w_y;
//	SEqDot_omega_3 = halfSEq_1 * w_y - halfSEq_2 * w_z + halfSEq_4 * w_x;
//	SEqDot_omega_4 = halfSEq_1 * w_z + halfSEq_2 * w_y - halfSEq_3 * w_x;
//	// compute then integrate the estimated quaternion rate
//	m_RotQuat.w += (SEqDot_omega_1 - (SF_Beta * SEqHatDot_1)) * deltat;
//	m_RotQuat.x += (SEqDot_omega_2 - (SF_Beta * SEqHatDot_2)) * deltat;
//	m_RotQuat.y += (SEqDot_omega_3 - (SF_Beta * SEqHatDot_3)) * deltat;
//	m_RotQuat.z += (SEqDot_omega_4 - (SF_Beta * SEqHatDot_4)) * deltat;
//	// normalise quaternion
//	norm = sqrt(m_RotQuat.w *  m_RotQuat.w + m_RotQuat.x *  m_RotQuat.x + m_RotQuat.y *  m_RotQuat.y + m_RotQuat.z *  m_RotQuat.z);
//	m_RotQuat.w /= norm;
//	m_RotQuat.x /= norm;
//	m_RotQuat.y /= norm;
//	m_RotQuat.z /= norm;
//	// compute flux in the earth frame
//	SEq_1SEq_2 = m_RotQuat.w *  m_RotQuat.x; // recompute axulirary variables
//	SEq_1SEq_3 = m_RotQuat.w *  m_RotQuat.y;
//	SEq_1SEq_4 = m_RotQuat.w *  m_RotQuat.z;
//	SEq_3SEq_4 = m_RotQuat.y *  m_RotQuat.z;
//	SEq_2SEq_3 = m_RotQuat.x *  m_RotQuat.y;
//	SEq_2SEq_4 = m_RotQuat.x *  m_RotQuat.z;
//	h_x = twom_x * (0.5f - m_RotQuat.y *  m_RotQuat.y - m_RotQuat.z *  m_RotQuat.z) + twom_y * (SEq_2SEq_3 - SEq_1SEq_4) + twom_z * (SEq_2SEq_4 + SEq_1SEq_3);
//	h_y = twom_x * (SEq_2SEq_3 + SEq_1SEq_4) + twom_y * (0.5f - m_RotQuat.x *  m_RotQuat.x - m_RotQuat.z *  m_RotQuat.z) + twom_z * (SEq_3SEq_4 - SEq_1SEq_2);
//	h_z = twom_x * (SEq_2SEq_4 - SEq_1SEq_3) + twom_y * (SEq_3SEq_4 + SEq_1SEq_2) + twom_z * (0.5f - m_RotQuat.x *  m_RotQuat.x - m_RotQuat.y *  m_RotQuat.y);
//	// normalise the flux vector to have only components in the x and z
//	b_x = sqrt((h_x * h_x) + (h_y * h_y));
//	b_z = h_z;
//}
