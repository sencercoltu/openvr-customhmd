#include "spi.h"
#include "led.h"

extern SPI_HandleTypeDef hspi1;

uint8_t snd_buffer[64];
uint8_t rcv_buffer[64];

void spi_waitStart(SPICommand &command)
{
	HAL_GPIO_WritePin(command.Port, command.Pin, GPIO_PIN_RESET); //chip enable for se8r01	
	//HAL_MicroDelay(50);
}

void spi_waitEnd(SPICommand &command)
{
	HAL_GPIO_WritePin(command.Port, command.Pin, GPIO_PIN_SET); //chip enable for se8r01
}


bool spi_rw(SPICommand &command)
{	
	spi_waitStart(command);	
	//bool result = HAL_SPI_TransmitReceive_DMA(&hspi1, &reg, pResult, 1) == HAL_OK;		
	bool result = HAL_SPI_TransmitReceive(command.pHandle, &command.Register, command.pResult, 1, (uint32_t)100) == HAL_OK;		
	spi_waitEnd(command);	
	return result;
}

bool spi_read(SPICommand &command)
{		
	command.pData[0] = 0;
	bool result = spi_rw_reg(command);		
	return result;	
}

bool spi_rw_reg(SPICommand &command)
{		
	snd_buffer[0] = command.Register; snd_buffer[1] = *command.pData;	
	spi_waitStart(command);	
	//bool result = HAL_SPI_TransmitReceive_DMA(&hspi1, snd_buffer, rcv_buffer, 2) == HAL_OK;		
	bool result = HAL_SPI_TransmitReceive(command.pHandle, snd_buffer, rcv_buffer, 2, (uint32_t)100) == HAL_OK;		
	spi_waitEnd(command);
	*command.pResult = rcv_buffer[1];
	return result;
	//return spi_rw(reg, pResult) & spi_rw(value, pResult);		
}

bool spi_write_buf(SPICommand &command)
{
	snd_buffer[0] = command.Register;	
	for (int i=0; i<command.DataSize; i++)
		snd_buffer[i+1] = command.pData[i];	
	spi_waitStart(command);	
	//bool result = HAL_SPI_TransmitReceive_DMA(&hspi1, snd_buffer, rcv_buffer, size + 1) == HAL_OK;		
	bool result = HAL_SPI_TransmitReceive(command.pHandle, snd_buffer, rcv_buffer, command.DataSize + 1, (uint32_t)100) == HAL_OK;			
	spi_waitEnd(command);
	*command.pResult = rcv_buffer[command.DataSize];
	return result;                  // return nRF24L01 status unsigned char
}

bool spi_read_buf(SPICommand &command)
{
	snd_buffer[0] = command.Register;	
	spi_waitStart(command);	
	//bool result = HAL_SPI_TransmitReceive_DMA(&hspi1, snd_buffer, rcv_buffer, size + 1) == HAL_OK;		
	bool result = HAL_SPI_TransmitReceive(command.pHandle, snd_buffer, rcv_buffer, command.DataSize + 1, (uint32_t)100) == HAL_OK;		
	spi_waitEnd(command);
	if (result)
		for (int i=0; i<command.DataSize; i++)
			command.pData[i] = rcv_buffer[i+1];	
	*command.pResult = rcv_buffer[command.DataSize];
	return result;                  // return nRF24L01 status unsigned char
}


