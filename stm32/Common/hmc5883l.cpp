#include "hmc5883l.h"

HMC5883L_Rate Mrate = MRT_75;        //  75 Hz ODR 

void initHMC5883L()
{
	// Set magnetomer ODR; default is 15 Hz 
	i2c_writeRegisterByte(HMC5883L_ADDRESS, HMC5883L_CONFIG_A, Mrate << 2);
	i2c_writeRegisterByte(HMC5883L_ADDRESS, HMC5883L_CONFIG_B, 0x00);  // set gain (bits[7:5]) to maximum resolution of 0.73 mG/LSB
	i2c_writeRegisterByte(HMC5883L_ADDRESS, HMC5883L_MODE, 0x80);     // enable continuous data mode
}

bool checkHMC5883L()
{
	LedOff();	
	if (i2c_isDeviceReady(HMC5883L_ADDRESS))
	{	
		uint8_t e, f, g;
		i2c_readRegister(HMC5883L_ADDRESS, HMC5883L_IDA, &e);  // Read WHO_AM_I register A for HMC5883L
		i2c_readRegister(HMC5883L_ADDRESS, HMC5883L_IDB, &f);  // Read WHO_AM_I register B for HMC5883L
		i2c_readRegister(HMC5883L_ADDRESS, HMC5883L_IDC, &g);  // Read WHO_AM_I register C for HMC5883L	

		if (e == 0x48 && f == 0x34 && g == 0x33)
			return true;
		BlinkDelay(10, 500);
	}	
	return false;
}

bool selfTestHMC5883L()
{
	int16_t selfTest[3] = { 0, 0, 0 };
	//  Perform self-test and calculate temperature compensation bias
	i2c_writeRegisterByte(HMC5883L_ADDRESS, HMC5883L_CONFIG_A, 0x71);   // set 8-average, 15 Hz default, positive self-test measurement
	i2c_writeRegisterByte(HMC5883L_ADDRESS, HMC5883L_CONFIG_B, 0xA0);   // set gain (bits[7:5]) to 5
	i2c_writeRegisterByte(HMC5883L_ADDRESS, HMC5883L_MODE, 0x80);      // enable continuous data mode
	HAL_Delay(150); // wait 150 ms

	uint8_t rawData[6] = { 0, 0, 0, 0, 0, 0 };                        // x/y/z gyro register data stored here
	i2c_readData(HMC5883L_ADDRESS, HMC5883L_OUT_X_H, rawData, 6);  // Read the six raw data registers sequentially into data array
	selfTest[0] = (((int16_t)rawData[0]) << 8) | rawData[1];          // Turn the MSB and LSB into a signed 16-bit value
	selfTest[1] = (((int16_t)rawData[4]) << 8) | rawData[5];
	selfTest[2] = (((int16_t)rawData[2]) << 8) | rawData[3];
	i2c_writeRegisterByte(HMC5883L_ADDRESS, HMC5883L_CONFIG_A, 0x00);   // exit self test

	if (selfTest[0] < 575 && selfTest[0] > 243 && selfTest[1] < 575 && selfTest[1] > 243 && selfTest[2] < 575 && selfTest[2] > 243)
	{
		return true;
	}
	else 
		return false; 
}

bool readHMC5883LData(int16_t *destination)
{
	uint8_t res = 0;
	if (i2c_readRegister(HMC5883L_ADDRESS, HMC5883L_STATUS, &res) && (res & 0x01))
	{
		uint8_t rawData[6];  // x/y/z gyro register data stored here
		i2c_readData(HMC5883L_ADDRESS, HMC5883L_OUT_X_H, rawData, 6);  // Read the six raw data registers sequentially into data array
		
		destination[0] = ((int16_t)rawData[0] << 8) | rawData[1];       // Turn the MSB and LSB into a signed 16-bit value
		destination[1] = ((int16_t)rawData[4] << 8) | rawData[5];
		destination[2] = ((int16_t)rawData[2] << 8) | rawData[3];

		return true;
	}
	return false;
}

float getHMC5883Lres()
{
	return 0.73;
}
