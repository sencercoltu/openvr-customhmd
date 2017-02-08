
#include <stdint.h>
#include <math.h>
#include "Quaternion.h"
#include "Kalman.h"
#include "Filters.h"

struct Sensor
{
	Sensor()
	{
		Gain = 1;
		Noise = 0;
		Resolution = 1;	
		for(int i=0; i<3; i++)
		{	
			Offset[i] = 0.0f;
			PosScale[i] = 
			NegScale[i] = 
			Deviation[i] = 1.0f;
			
			Raw[i] = 
			LastRaw[i] = 0;
			Filtered[i] = 0.0f;
		}		
	}
	
	void ResetCalibration()
	{
		for(int i=0; i<3; i++)
		{	
			Offset[i] = 0.0f;
			PosScale[i] = 
			NegScale[i] = 1.0f;
		}
	}
	
	void SetResolution(float res)
	{
		Resolution = res;
	}

	void SetNoise(float noise)
	{
		Noise = noise;
	}

	void SetGain(float gain)
	{
		Gain = gain;
		if (Gain > 1) Gain = 1;
		if (Gain < 0) Gain = 0;			
	}
	
	float Gain;
	float Noise;
	float Resolution;
	float Offset[3];
	float PosScale[3]; 
	float NegScale[3];
	float Deviation[3];

	int16_t Raw[3]; 
	int16_t LastRaw[3];
	//float Max[3];
	//float Min[3];
	float Filtered[3]; 
	float Compensated[3];	
	float Converted[3];	

	void ProcessNew()
	{
		for (int i=0; i<3; i++)
		{
			float offsetCompensated = Raw[i] - Offset[i];
			float rawDiff = LastRaw[i] - offsetCompensated;
			rawDiff  = fabs(rawDiff);
			if (rawDiff > Noise)
				rawDiff = Noise; 
			Deviation[i] = Deviation[i] * (0.9f) + rawDiff * (0.1f);
			Compensated[i] = offsetCompensated * ((offsetCompensated > 0) ? PosScale[i] : NegScale[i]);
			LastRaw[i] = offsetCompensated;			
			if (fabs(Filtered[i] - Compensated[i]) > Deviation[i])
				Filtered[i] = Compensated[i];
			else
				Filtered[i] = Filtered[i] * (1.0f - Gain) +  Compensated[i] * Gain;			
			Converted[i] = Filtered[i] * Resolution;
		}
	}
	
//	float FixScale(float val, float pos, float neg)
//	{
//		if (val > 0)
//			return val * pos;
//		else if (val < 0)
//			return val * neg;
//		return 0;
//	}	
};

struct SensorData
{
	void ResetCalibration()
	{		
		Accel.ResetCalibration();
		Gyro.ResetCalibration();
		Mag.ResetCalibration();
	}
	
	void Setup(float aRes, float gRes, float mRes)
	{
		Accel.SetResolution(aRes);
		Accel.SetNoise(10);
		Accel.SetGain(0.5);
		Gyro.SetResolution(gRes);
		Gyro.SetGain(0.1);		
		Gyro.SetNoise(100);
		Mag.SetResolution(mRes);
	}
	
	float TimeElapsed; 
	
	Sensor Accel;
	Sensor Gyro;
	Sensor Mag;
	
//	int16_t Accel[3]; 
//	float AccelMax[3];
//	float AccelMin[3];
//	float AccelFiltered[3]; 
//	float PosScaleAccel[3]; 
//	float NegScaleAccel[3]; 	
//	
//	
//	
//	int16_t Gyro[3];
//	int16_t Mag[3];
//	
//	float PosScaleGyro[3];
//	float NegScaleGyro[3];	
//	float PosScaleMag[3];
//	float NegScaleMag[3];		
//	
//	
//	
//	float GyroFiltered[3];
//	float MagFiltered[3];
	
	
	
	//KalmanSingle KalmanAccel[3];
	//KalmanSingle KalmanVelocity[3];
	//KalmanSingle KalmanPosition[3];
	
//	float FixScale(float val, float pos, float neg)
//	{
//		if (val > 0)
//			return val * pos;
//		else if (val < 0)
//			return val * neg;
//		return 0;
//	}

//	KalmanSingle KalmanVelocity[3];
	
//	Filter HighPassPosition[3];
//	Filter HighPassVelocity[3];
}; 


//#define SF_GyroMeasError M_PI * (40.0f / 180.0f)       // gyroscope measurement error in rads/s (shown as 40 deg/s)
//#define SF_GyroMeasError M_PI * (40.0f / 180.0f)       // gyroscope measurement error in rads/s (shown as 40 deg/s)
//#define SF_GyroMeasDrift M_PI * (0.1f / 180.0f)      // gyroscope measurement drift in rad/s/s (shown as 0.0 deg/s/s)
//#define SF_Beta sqrt(3.0f / 4.0f) * SF_GyroMeasError // compute beta
//#define SF_Beta 0.2f //1.3f
//#define SF_Zeta sqrt(3.0f / 4.0f) * SF_GyroMeasDrift // compute zeta
//#define SF_Kp 2.0f * 5.0f // these are the free parameters in the Mahony filter and fusion scheme, Kp for proportional feedback, Ki for integral
//#define SF_Ki 0.001f
#define RAD(a) (a*M_PI/180.0)

class CSensorFusion
{
public:
	CSensorFusion(float beta);
	~CSensorFusion();
	Quaternion FuseOrient(SensorData *pData);
	Quaternion FuseGrav(SensorData *pData);
	Quaternion m_RotQuat;	
//	static Quaternion Fix(const Quaternion &quat);    
private:
	float m_Beta;
	float m_aR, m_mR, m_gR;
	float exInt, eyInt, ezInt;
	float iq0, iq1, iq2, iq3;	
	void MadgwickQuaternionUpdateAHRS(float deltat, float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);
	void MadgwickQuaternionUpdateIMU(float deltat, float ax, float ay, float az, float gx, float gy, float gz);	
};

extern float invSqrt(float x);
extern void SmoothSensorData(float *newData, float *oldData, float mag);

