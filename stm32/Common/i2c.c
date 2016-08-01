#include "i2c.h"
#include "led.h"

extern I2C_HandleTypeDef hi2c2;

bool i2c_writeSingleByte(uint8_t addr, uint8_t data)
{	
	if (HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)addr << 1, &data, 1, (uint8_t) 100) != HAL_OK)	
		return false;
	return true;
}

bool i2c_writeRegisterByte(uint8_t addr, uint8_t reg, uint8_t data)
{	
	if (HAL_I2C_Mem_Write(&hi2c2, (uint16_t)addr << 1, (uint16_t) reg, I2C_MEMADD_SIZE_8BIT, &data, 1, (uint8_t) 100) != HAL_OK)	
		return false;
	return true;
}

bool i2c_readRegister(uint8_t addr, uint8_t reg, uint8_t* pResult)
{	
	return i2c_readData(addr, reg, pResult, 1);
}

bool i2c_readData(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size)
{	
	if (HAL_I2C_Mem_Read(&hi2c2, (uint16_t)addr << 1, (uint16_t) reg, I2C_MEMADD_SIZE_8BIT, buffer, size, (uint8_t) 100) != HAL_OK)	
		return false;
	return true;
}

bool i2c_isDeviceReady(uint8_t addr)
{
	BlinkRease(30);
	if (HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)addr << 1, 5, (uint32_t) 1000) != HAL_OK)
		return false;	
	LedOn();
	HAL_Delay(250);
	return true;
}

