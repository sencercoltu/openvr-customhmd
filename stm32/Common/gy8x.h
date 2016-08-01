#include "adxl345.h"
#include "l3g4200d.h"
#include "hmc5883l.h"
#include "itg3200.h"
#include "bmx055.h"

float gRes = 1.0f;
float mRes = 1.0f;
float aRes = 1.0f;
int16_t gyroOffset[3] = {0};


bool hasADXL345 = false;
bool hasL3G4200D = false;
bool hasHMC5883L = false;
bool hasITG3200 = false;
bool hasBMX055 = false;

bool readAccelData(int16_t *destination)
{
	bool ret = false;
#ifdef SENSOR_SMOOTHING
	int16_t temp[3];
	*temp = *destination;
#endif
	if (hasBMX055)
		ret = readBMX055DataAccel(destination);
	else if (hasADXL345)
		ret = readADXL345Data(destination);
#ifdef SENSOR_SMOOTHING
	if (ret)
	{
		SmoothSensorData(temp, destination, aRes);
	}
#endif
	return ret;
}

bool readGyroData(int16_t *destination)
{
	bool ret = false;
#ifdef SENSOR_SMOOTHING
	int16_t temp[3];
	*temp = *destination;
#endif
	if (hasBMX055)		
		ret = readBMX055DataGyro(destination);
	else if (hasL3G4200D)
		ret = readL3G4200DData(destination);
	else if (hasITG3200)
		ret = readITG3200Data(destination);
#ifdef SENSOR_SMOOTHING
	if (ret)
	{
		SmoothSensorData(temp, destination, gRes);
	}
#endif
	return ret;
}

void resetGyroOffset()
{
}

void calibrateGyroOffset()
{	
	resetGyroOffset();
	int readings = 0;	
	int16_t data[3] = {0};		
	do
	{
		if (readGyroData(data))
		{			
			if (!readings)
			{
				gyroOffset[0] = data[0];
				gyroOffset[1] = data[1];
				gyroOffset[2] = data[2];
			}
			else
			{			
				gyroOffset[0] = gyroOffset[0] * 0.95 + data[0] * 0.05;
				gyroOffset[1] = gyroOffset[1] * 0.95 + data[1] * 0.05;
				gyroOffset[2] = gyroOffset[2] * 0.95 + data[2] * 0.05;
			}
			readings++;
		}		
	} while(readings < 100);
}



bool readMagData(int16_t *destination)
{
	bool ret = false;
#ifdef SENSOR_SMOOTHING
	int16_t temp[3];
	*temp = *destination;
#endif
	if (hasBMX055)		
		ret = readBMX055DataMag(destination);
	else if (hasHMC5883L) 
		ret = readHMC5883LData(destination);
#ifdef SENSOR_SMOOTHING
	if (ret)
	{
		SmoothSensorData(temp, destination, gRes);
	}
#endif
	return ret;
}

void getGres() 
{
	if (hasBMX055) gRes = getBMX055resGyro();
	else if (hasL3G4200D) gRes = getL3G4200Dres();
	else if (hasITG3200) gRes = getITG3200res();
	else gRes = 0;
}

void getAres() 
{
	if (hasBMX055) aRes = getBMX055resAccel();
	else if (hasADXL345) aRes = getADXL345res();
	else aRes = 0;
}

void getMres() 
{
	if (hasBMX055) mRes = getBMX055resMag();
	else if (hasHMC5883L) mRes = getHMC5883Lres();
	else mRes = 0;
}


bool initSensors()
{
	hasBMX055 = checkBMX055(); if (hasBMX055) {initBMX055();}
	if (!hasBMX055)
	{
		hasADXL345 = checkADXL345(); if (hasADXL345) {initADXL345();}
		hasHMC5883L = checkHMC5883L(); if (hasHMC5883L) initHMC5883L();
		hasL3G4200D = checkL3G4200D(); if (hasL3G4200D) initL3G4200D();
		if (!hasL3G4200D) //same address as hasITG3200
			hasITG3200 = checkITG3200(); if (hasITG3200) initITG3200();
	}
	getAres();
	getGres();
	getMres();
	
	return (aRes && gRes && mRes);
}


