#include "stm32f1xx_hal.h"

#ifndef _SPI_H_
#define _SPI_H_

struct SPICommand
{
	SPI_HandleTypeDef *pHandle;
	GPIO_TypeDef *Port;
	uint16_t Pin;
	uint8_t Register;
	uint8_t *pData;
	uint8_t DataSize;
	uint8_t *pResult;
};

extern bool spi_rw(SPICommand &pCommand);
extern bool spi_read(SPICommand &command);
extern bool spi_rw_reg(SPICommand &command);
extern bool spi_write_buf(SPICommand &command);
extern bool spi_read_buf(SPICommand &command);

#endif  //_SPI_H_
