#include "itg3200.h"

bool checkITG3200()
{
	LedOff();	
	if (i2c_isDeviceReady(ITG3200_ADDRESS))
	{	
		uint8_t d = 0;
		i2c_readRegister(ITG3200_ADDRESS, WHO_AM_I_ITG3200, &d);
		if ((d & 0x7E) == 0x68)
			return true;
		BlinkDelay(10, 500);
	}	
	return false;	
}

void initITG3200()
{
	i2c_writeRegisterByte(ITG3200_ADDRESS, ITG3200_RA_PWR_MGM, 0x81);
	i2c_writeRegisterByte(ITG3200_ADDRESS, ITG3200_RA_SMPLRT_DIV, 0x01);
	i2c_writeRegisterByte(ITG3200_ADDRESS, ITG3200_RA_DLPF_FS, 0x19);
	i2c_writeRegisterByte(ITG3200_ADDRESS, ITG3200_RA_INT_CFG, 0x00);	
}

bool readITG3200Data(int16_t *destination)
{
	//uint8_t res = 0;
	//if (i2c_readRegister(ITG3200_ADDRESS, L3G4200D_STATUS_REG, &res) && (res & 0x80))
	//{	
		uint8_t rawData[8];  // x/y/z gyro register data stored here
		i2c_readData(ITG3200_ADDRESS, ITG3200_RA_TEMP_OUT_H, rawData, 8);  // Read the six raw data registers sequentially into data array
		destination[0] = (((int16_t)rawData[2]) << 8) | rawData[3];  // Turn the MSB and LSB into a signed 16-bit value
		destination[1] = (((int16_t)rawData[4]) << 8) | rawData[5];
		destination[2] = (((int16_t)rawData[6]) << 8) | rawData[7];
		return true;
	//}
	//return false;
}

float getITG3200res()
{
	return 1.0f / 14.375f;		
}
