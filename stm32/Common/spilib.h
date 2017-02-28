#ifndef _SPILIB_H_
#define _SPILIB_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "stm32f1xx_hal.h"

extern uint8_t SPI_SendByte(SPI_HandleTypeDef* SPIx, uint8_t cmd);
extern uint8_t SPI_ReadByte(SPI_HandleTypeDef* SPIx, uint8_t reg);

#ifdef __cplusplus
}
#endif

#endif //_SPILIB_H_
