

#pragma once
#define _USE_MATH_DEFINES

#include "Common.h"
#include <math.h>
#include "Quaternion.h"

#pragma pack(push)
#pragma pack(1)
typedef struct
{
	float TimeElapsed; //4
	float Accel[3]; //2*3=6
	float Gyro[3]; //2*3=6
	float Mag[3]; //2*3=6	
} SensorData; //total 34
#pragma pack(pop)


#define SF_GyroMeasError M_PI * (5.0f / 180.0f)       // gyroscope measurement error in rads/s (shown as 40 deg/s)
#define SF_GyroMeasDrift M_PI * (0.2f / 180.0f)      // gyroscope measurement drift in rad/s/s (shown as 0.0 deg/s/s)
#define SF_Beta sqrt(3.0f / 4.0f) * SF_GyroMeasError // compute beta
#define SF_Zeta sqrt(3.0f / 4.0f) * SF_GyroMeasDrift // compute zeta
#define SF_Kp 2.0f * 5.0f // these are the free parameters in the Mahony filter and fusion scheme, Kp for proportional feedback, Ki for integral
#define SF_Ki 0.0f
#define RAD(a) (a*M_PI/180.0)

class CSensorFusion
{
public:
	CSensorFusion();
	~CSensorFusion();
	Quaternion Fuse(SensorData *pData);
	Quaternion Value();
	//static vr::HmdQuaternion_t QuatMultiply(const vr::HmdQuaternion_t *q1, const vr::HmdQuaternion_t *q2);
	//static vr::HmdQuaternion_t QuatNormalize(const vr::HmdQuaternion_t *q);
	//static vr::HmdQuaternion_t QuatConjugate(const vr::HmdQuaternion_t *q);
	//static vr::HmdQuaternion_t QuatInverse(const vr::HmdQuaternion_t *q);
	

	//static vr::HmdVector3d_t QuaterionToEuler(const vr::HmdQuaternion_t *q);
	//static vr::HmdQuaternion_t EulerToQuaternion(const vr::HmdVector3d_t *e);



	static void HmdMatrix_SetIdentity(vr::HmdMatrix34_t *pMatrix);
private:
	Quaternion m_RotQuat;
	//double eInt[3];
	//double b_x, b_z; // reference direction of flux in earth frame
	//double w_bx, w_by, w_bz; // estimate gyroscope biases error
	void MadgwickQuaternionUpdate(double deltat, double ax, double ay, double az, double gx, double gy, double gz, double mx, double my, double mz);
	//void MahonyQuaternionUpdate(double deltat, double ax, double ay, double az, double gx, double gy, double gz, double mx, double my, double mz);
	//void FilterUpdate(double deltat, double a_x, double a_y, double a_z, double w_x, double w_y, double w_z, double m_x, double m_y, double m_z);
};

