#include "nunchuck.h"
#include "led.h"

uint8_t nunchuckId[6] = {0};


void initNunchuck()
{		
	uint8_t dummy = 0;
	i2c_readRegister(NUNCHUCK_ADDRESS, 0x00, &dummy);
	HAL_Delay(10);
	if (!i2c_writeRegisterByte(NUNCHUCK_ADDRESS, 0xF0, 0x55)) return;
	HAL_Delay(10);
	if (!i2c_writeRegisterByte(NUNCHUCK_ADDRESS, 0xFB, 0x00))  return;	
	HAL_Delay(10);
	if (!i2c_writeSingleByte(NUNCHUCK_ADDRESS, 0xfa)) return;
	HAL_Delay(10);
	if (!i2c_readData(NUNCHUCK_ADDRESS, nunchuckId, 6)) return;			
	HAL_Delay(10);
	if (nunchuckId[0] == 0 &&
		nunchuckId[1] == 0 &&
		nunchuckId[2] == 0xa4 &&
		nunchuckId[3] == 0x20 &&
		nunchuckId[4] == 0 &&
		nunchuckId[5] == 0)			
			BlinkDelay(10, 20);			
		else
			BlinkDelay(10, 500);
}



void readNunchuck(uint8_t *data)
{
	if (!i2c_writeSingleByte(NUNCHUCK_ADDRESS, 0x00)) return;	
	HAL_Delay(5);
	if (!i2c_readData(NUNCHUCK_ADDRESS, data, 6)) return;
	//for (int i=0; i<6; i++)
	//	data[i] = nunchuk_decode_byte(data[i]);
	//i2c_writeRegisterByte(NUNCHUCK_ADDRESS, 0x00, 0x00);
}

uint8_t nunchuk_decode_byte(uint8_t x)
{
	return (x ^ 0x17) + 0x17; 
}
