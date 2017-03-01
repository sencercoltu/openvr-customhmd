#include "spilib.h"

uint8_t SPI_SendByte(SPI_HandleTypeDef* SPIx, uint8_t cmd)
{
	uint8_t status = 0;
	if (HAL_SPI_TransmitReceive(SPIx, &cmd, &status, 1, (uint32_t)100) == HAL_OK)
		return status;
	return 0;
}

uint8_t SPI_ReadByte(SPI_HandleTypeDef* SPIx, uint8_t reg)
{
	uint8_t data = 0xFF;
	if (HAL_SPI_TransmitReceive(SPIx, &reg, &data, 1, (uint32_t)100) == HAL_OK)
		return data;
	return 0xFF;
}
