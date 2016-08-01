#include "stm32f1xx_hal.h"

extern uint8_t SPI_SendByte(SPI_HandleTypeDef* SPIx, uint8_t cmd);
extern uint8_t SPI_ReadByte(SPI_HandleTypeDef* SPIx, uint8_t reg);
