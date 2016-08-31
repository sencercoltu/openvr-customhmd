
#include <stdint.h>
#include <math.h>
#include "Quaternion.h"

#pragma pack(push)
#pragma pack(1)
typedef struct
{
	float TimeElapsed; 
	int16_t Accel[3]; 
	int16_t Gyro[3];
	int16_t Mag[3];
	
	int16_t OffsetAccel[3]; 
	int16_t OffsetGyro[3];
	int16_t OffsetMag[3];	
	
} SensorData; 
#pragma pack(pop)


#define SF_GyroMeasError M_PI * (40.0f / 180.0f)       // gyroscope measurement error in rads/s (shown as 40 deg/s)
#define SF_GyroMeasDrift M_PI * (0.0f / 180.0f)      // gyroscope measurement drift in rad/s/s (shown as 0.0 deg/s/s)
#define SF_Beta sqrt(3.0f / 4.0f) * SF_GyroMeasError // compute beta
#define SF_Zeta sqrt(3.0f / 4.0f) * SF_GyroMeasDrift // compute zeta
#define SF_Kp 2.0f * 5.0f // these are the free parameters in the Mahony filter and fusion scheme, Kp for proportional feedback, Ki for integral
#define SF_Ki 0.005f
#define RAD(a) (a*M_PI/180.0)

class CSensorFusion
{
public:
	CSensorFusion(float aR, float gR, float mR);
	~CSensorFusion();
	Quaternion Fuse(SensorData *pData);
	Quaternion Value();
private:
	float m_aR, m_mR, m_gR;
	float exInt, eyInt, ezInt;
	float iq0, iq1, iq2, iq3;
	Quaternion m_RotQuat;
	void MadgwickQuaternionUpdate(float deltat, float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);
};

extern void SmoothSensorData(float *newData, float *oldData, float mag);

