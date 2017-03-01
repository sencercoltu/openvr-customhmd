#include "l3g4200d.h"

L3G4200D_Scale L3G4200DGscale = GFS_250DPS;
L3G4200D_Rate L3G4200DGrate = GRTBW_200_50;  // 200 Hz ODR,  50 Hz bandwidth


void initL3G4200D()
{
	// Set gyro ODR to 100 Hz and Bandwidth to 25 Hz, enable normal mode
	i2c_writeRegisterByte(L3G4200D_ADDRESS, L3G4200D_CTRL_REG1, L3G4200DGrate << 4 | 0x0F);  //start
	i2c_writeRegisterByte(L3G4200D_ADDRESS, L3G4200D_CTRL_REG3, 0x08);        // Push/pull, active high interrupt, enable int1 
	i2c_writeRegisterByte(L3G4200D_ADDRESS, L3G4200D_CTRL_REG4, L3G4200DGscale << 4); // set gyro full scale
	i2c_writeRegisterByte(L3G4200D_ADDRESS, L3G4200D_FIFO_CTRL_REG, 0x00);
}

bool checkL3G4200D()
{
	LedOff();	
	if (i2c_isDeviceReady(L3G4200D_ADDRESS))
	{	
		uint8_t d = 0;
		i2c_readRegister(L3G4200D_ADDRESS, WHO_AM_I_L3G4200D, &d);
		if (d == 0xD3)
			return true;
		BlinkDelay(10, 500);
	}	
	return false;
}


bool readL3G4200DData(int16_t *destination)
{
	uint8_t res = 0;
	if (i2c_readRegister(L3G4200D_ADDRESS, L3G4200D_STATUS_REG, &res) && (res & 0x80))
	{	
		uint8_t rawData[6];  // x/y/z gyro register data stored here
		i2c_readData(L3G4200D_ADDRESS, L3G4200D_OUT_X_L | 0x80, rawData, 6);  // Read the six raw data registers sequentially into data array
		destination[0] = (((int16_t)rawData[1]) << 8) | rawData[0];  // Turn the MSB and LSB into a signed 16-bit value
		destination[1] = (((int16_t)rawData[3]) << 8) | rawData[1];
		destination[2] = (((int16_t)rawData[5]) << 8) | rawData[4];
		return true;
	}
	return false;
}

float getL3G4200Dres()
{
	switch (L3G4200DGscale)
	{
		// Possible gyro scales (and their register bit settings) are:
		// 250 DPS (00), 500 DPS (01), 1000 DPS (10), and 2000 DPS  (11). 
	case GFS_250DPS:
		return 250.0f / 32768.0f;
	case GFS_500DPS:
		return 500.0f / 32768.0f;
	case GFS_1000DPS:
		return 1000.0f / 32768.0f;		
	case GFS_2000DPS:
		return 2000.0f / 32768.0f;		
	}
	return 0.0f;
}
