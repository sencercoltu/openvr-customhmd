#include "se8r01.h"
#include "led.h"
#include "usb.h"

//SE8R01_RFMode SE8R01_OPERATE_MODE = SE8R01_RFMode_Receive;
SE8R01_RFMode SE8R01_OPERATE_MODE = SE8R01_RFMode_Transmit;

#define TX_ADR_WIDTH    5   // 5 unsigned chars TX(RX) address width
#define TX_PLOAD_WIDTH  32  // 32 unsigned chars TX payload

unsigned char TX_ADDRESS1[TX_ADR_WIDTH]  = 
{
  'S','E','N','C', 0x01
}; // Define a static TX address

unsigned char TX_ADDRESS2[TX_ADR_WIDTH]  = 
{
  'S','E','N','C', 0x02
}; // Define a static TX address

SPICommand rfSPICommand = {0};


bool SE8R01_ClearInterrupts()
{	
	uint8_t status;	
	uint8_t value = 0xff;
	rfSPICommand.Register = iRF_CMD_WRITE_REG | iRF_BANK0_STATUS;
	rfSPICommand.pData = &value;
	rfSPICommand.DataSize = 1;
	rfSPICommand.pResult = &status;
	return spi_rw_reg(rfSPICommand);
}

bool SE8R01_Init() 
{
	rfSPICommand.pHandle = &hspi1;
	rfSPICommand.Pin = SPI1_RF_NSS_Pin;
	rfSPICommand.Port = SPI1_RF_NSS_GPIO_Port;		
	uint8_t status;	
	rfSPICommand.pResult = &status;
	uint8_t value;
	rfSPICommand.pData = &value;	
	rfSPICommand.DataSize = 1;
	uint8_t gtemp[5];	

	BlinkRease(30);
		
	HAL_GPIO_WritePin(SPI1_RF_CE_GPIO_Port, SPI1_RF_CE_Pin, GPIO_PIN_RESET); //disable rf
	HAL_GPIO_WritePin(SPI1_RF_NSS_GPIO_Port, SPI1_RF_NSS_Pin, GPIO_PIN_SET); //disable rf
	
	//HAL_GPIO_WritePin(SPI1_CE_GPIO_Port, SPI1_CE_Pin, GPIO_PIN_RESET); //chip enable for se8r01
	//HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_SET); //disable comm at startup
	//HAL_GPIO_WritePin(SPI1_INT_GPIO_Port, SPI1_INT_Pin, GPIO_PIN_RESET);
	
	bool result = SE8R01_SwitchBank(iBANK0);	
	
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_CONFIG; value = 0xa7; result &= spi_rw_reg(rfSPICommand); HAL_Delay(100); //power down
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_CONFIG; value = 0xa7; result &= spi_rw_reg(rfSPICommand); HAL_Delay(200);
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_RF_CH; value = 0x32; result &= spi_rw_reg(rfSPICommand);
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_RF_SETUP; value = 0x48; result &= spi_rw_reg(rfSPICommand);	
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_PRE_GURD; value = 0x77; result &= spi_rw_reg(rfSPICommand); //2450 calibration	

	//calibration
	
	result = SE8R01_SwitchBank(iBANK1);
	
	rfSPICommand.pData = gtemp;

	gtemp[0]=0x40; gtemp[1]=0x00; gtemp[2]=0x10; gtemp[3]=0xE6;	
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_PLL_CTL0; rfSPICommand.DataSize = 4; result &= spi_write_buf(rfSPICommand);

	gtemp[0]=0x20;	gtemp[1]=0x08;	gtemp[2]=0x50;	gtemp[3]=0x40;	gtemp[4]=0x50;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_CAL_CTL; rfSPICommand.DataSize = 5; result &= spi_write_buf(rfSPICommand);

	gtemp[0]=0x00;	gtemp[1]=0x00;	gtemp[2]=0x1E;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_IF_FREQ; rfSPICommand.DataSize = 3; result &= spi_write_buf(rfSPICommand);

	gtemp[0]=0x29;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_FDEV; rfSPICommand.DataSize = 1; result &= spi_write_buf(rfSPICommand);	

	gtemp[0]=0x7F;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_DAC_CAL_HI; rfSPICommand.DataSize = 1; result &= spi_write_buf(rfSPICommand);		

	gtemp[0]=0x02;	gtemp[1]=0xC1;	gtemp[2]=0xEB;	gtemp[3]=0x1C;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_AGC_GAIN; rfSPICommand.DataSize = 4; result &= spi_write_buf(rfSPICommand);		

	gtemp[0]=0x97;	gtemp[1]=0x64;	gtemp[2]=0x00;	gtemp[3]=0x81;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_RF_IVGEN; rfSPICommand.DataSize = 4; result &= spi_write_buf(rfSPICommand);	

	gtemp[0]=0x00;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_DAC_CAL_LOW; rfSPICommand.DataSize = 1; result &= spi_write_buf(rfSPICommand);	
	
	result &= SE8R01_SwitchBank(iBANK0);
	
	rfSPICommand.pData = &value;
	rfSPICommand.DataSize = 1;

	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_CONFIG; value = 0x03; result &= spi_rw_reg(rfSPICommand); HAL_Delay(15);	
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_CONFIG; value = 0x01; result &= spi_rw_reg(rfSPICommand); HAL_Delay(25);
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_CONFIG; value = 0x03; result &= spi_rw_reg(rfSPICommand); HAL_Delay(15);
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_CONFIG; value = 0x01; result &= spi_rw_reg(rfSPICommand); HAL_Delay(25);
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_CONFIG; value = 0x03; result &= spi_rw_reg(rfSPICommand); 

//	
//	

//	HAL_GPIO_WritePin(SPI1_CE_GPIO_Port, SPI1_CE_Pin, GPIO_PIN_SET);
//	HAL_MicroDelay(30);
//	HAL_GPIO_WritePin(SPI1_CE_GPIO_Port, SPI1_CE_Pin, GPIO_PIN_RESET);

//	HAL_MicroDelay(50);                            // delay 50ms waitting for calibaration.

//	HAL_GPIO_WritePin(SPI1_CE_GPIO_Port, SPI1_CE_Pin, GPIO_PIN_SET);
//	HAL_MicroDelay(30);
//	HAL_GPIO_WritePin(SPI1_CE_GPIO_Port, SPI1_CE_Pin, GPIO_PIN_RESET);

//	HAL_MicroDelay(50);                            // delay 50ms waitting for calibaration.
//	// calibration end	
		
		
		
	//setup
	rfSPICommand.pData = gtemp;
	
	gtemp[0]=0x28;	gtemp[1]=0x32;	gtemp[2]=0x80;	gtemp[3]=0x90;	gtemp[4]=0x00;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_SETUP_VALUE; rfSPICommand.DataSize = 5; result &= spi_write_buf(rfSPICommand); HAL_Delay(1);

	result &= SE8R01_SwitchBank(iBANK1);

	gtemp[0]=0x40;	gtemp[1]=0x01;	gtemp[2]=0x30;	gtemp[3]=0xE2;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_PLL_CTL0; rfSPICommand.DataSize = 4; result &= spi_write_buf(rfSPICommand);
	
	gtemp[0]=0x29;	gtemp[1]=0x89;	gtemp[2]=0x55;	gtemp[3]=0x40;	gtemp[4]=0x50;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_CAL_CTL; rfSPICommand.DataSize = 5; result &= spi_write_buf(rfSPICommand);

	gtemp[0]=0x29;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_FDEV; rfSPICommand.DataSize = 1; result &= spi_write_buf(rfSPICommand);

	gtemp[0]=0x55;	gtemp[1]=0xC2;	gtemp[2]=0x09;	gtemp[3]=0xAC;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_RX_CTRL; rfSPICommand.DataSize = 4; result &= spi_write_buf(rfSPICommand);
	
	gtemp[0]=0x00;	gtemp[1]=0x14;	gtemp[2]=0x08;	gtemp[3]=0x29;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_FAGC_CTRL_1; rfSPICommand.DataSize = 4; result &= spi_write_buf(rfSPICommand);	

	gtemp[0]=0x02;	gtemp[1]=0xC1;	gtemp[2]=0xCB;	gtemp[3]=0x1C;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_AGC_GAIN; rfSPICommand.DataSize = 4; result &= spi_write_buf(rfSPICommand);	
	
	gtemp[0]=0x97;	gtemp[1]=0x64;	gtemp[2]=0x00;	gtemp[3]=0x01;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_RF_IVGEN; rfSPICommand.DataSize = 4; result &= spi_write_buf(rfSPICommand);		

	gtemp[0]=0x2A;	gtemp[1]=0x04;	gtemp[2]=0x00;	gtemp[3]=0x7D;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK1_TEST_PKDET; rfSPICommand.DataSize = 4; result &= spi_write_buf(rfSPICommand);		
	
	result &= SE8R01_SwitchBank(iBANK0);		
	
	//radio settings
	rfSPICommand.pData = &value;	
	rfSPICommand.DataSize = 1;
	
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_SETUP_AW; value = 0x03; result &= spi_rw_reg(rfSPICommand); 	
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_SETUP_RETR; value = 0x0a; result &= spi_rw_reg(rfSPICommand); //lowest 4 bits 0-15 rt transmisston higest 4 bits 256-4096us Auto Retransmit Delay		
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_RF_CH; value = 0x11; result &= spi_rw_reg(rfSPICommand); 		
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_RF_SETUP; value = 0x4f; result &= spi_rw_reg(rfSPICommand); //0x4f //1mps 0x48	


	if (SE8R01_OPERATE_MODE == SE8R01_RFMode_Receive)
	{
		rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_EN_AA; value = 0x03; result &= spi_rw_reg(rfSPICommand); //0x01 //enable auto acc on pip 1 and 2
		rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_EN_RXADDR; value = 0x03; result &= spi_rw_reg(rfSPICommand); //enable pip 1 and 2		
		rfSPICommand.pData = TX_ADDRESS1; rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_RX_ADDR_P0; rfSPICommand.DataSize = TX_ADR_WIDTH; result &= spi_write_buf(rfSPICommand); // Use the same address on the RX device as the TX device
		rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_TX_ADDR; result &= spi_write_buf(rfSPICommand); 		
		rfSPICommand.pData = &TX_ADDRESS2[4]; rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_RX_ADDR_P1; rfSPICommand.DataSize = 1; result &= spi_write_buf(rfSPICommand);		
	}
	else
	{
		rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_EN_AA; value = 0x01; result &= spi_rw_reg(rfSPICommand); //0x01 //enable auto acc on pip 1		
		rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_EN_RXADDR; value = 0x01; result &= spi_rw_reg(rfSPICommand); //enable pip 1 and 2		
		
		if (ctlSource == LEFTCTL_SOURCE)
		{
			rfSPICommand.pData = TX_ADDRESS1;rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_RX_ADDR_P0; rfSPICommand.DataSize = TX_ADR_WIDTH; result &= spi_write_buf(rfSPICommand);
			rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_TX_ADDR; result &= spi_write_buf(rfSPICommand); 
		}
		else if (ctlSource == RIGHTCTL_SOURCE)
		{
			rfSPICommand.pData = TX_ADDRESS2;rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_RX_ADDR_P0; rfSPICommand.DataSize = TX_ADR_WIDTH; result &= spi_write_buf(rfSPICommand);
			rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_TX_ADDR; result &= spi_write_buf(rfSPICommand); 
		}
	}
	
	rfSPICommand.pData = &value;	
	rfSPICommand.DataSize = 1;
	
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_RX_PW_P0; value = TX_PLOAD_WIDTH; result &= spi_rw_reg(rfSPICommand); 	// Select same RX payload width as TX Payload width	

	uint8_t reg = (SE8R01_OPERATE_MODE == SE8R01_RFMode_Receive)? 0x3f : 0x4e;
	rfSPICommand.Register = iRF_CMD_WRITE_REG|iRF_BANK0_CONFIG; value = reg; result &= spi_rw_reg(rfSPICommand);
	
	rfSPICommand.pResult = &value;
	rfSPICommand.Register = iRF_BANK0_CONFIG; value = 0; result &= spi_rw_reg(rfSPICommand);
	
	result &= value == reg;

	if (!result)
	{
		BlinkDelay(500, 10);
		HAL_GPIO_WritePin(SPI1_RF_CE_GPIO_Port, SPI1_RF_CE_Pin, GPIO_PIN_RESET); //chip enable for se8r01
		HAL_GPIO_WritePin(SPI1_RF_NSS_GPIO_Port, SPI1_RF_NSS_Pin, GPIO_PIN_SET); //disable comm at startup
	}
	else
	{
		SE8R01_FlushBuffers();
		SE8R01_ClearInterrupts();
		HAL_GPIO_WritePin(SPI1_RF_CE_GPIO_Port, SPI1_RF_CE_Pin, GPIO_PIN_SET); //chip enable for se8r01
		LedOn();
	}
	return result;		
}

bool SE8R01_SwitchBank(uint8_t bankindex)
{
	uint8_t status;	
	rfSPICommand.pResult = &status;
	uint8_t value;
	rfSPICommand.pData = &value;	
	rfSPICommand.DataSize = 1;
	rfSPICommand.Register = iRF_BANK0_STATUS;
	spi_rw(rfSPICommand);
    if((status&0x80) != bankindex)
	{
		value = 0x53;
		rfSPICommand.Register = iRF_CMD_ACTIVATE;
		return spi_rw_reg(rfSPICommand);
	}
	return true;
}

bool SE8R01_ReadPayload(uint8_t* pBuf)
{
	uint8_t status;	
	rfSPICommand.pResult = &status;	
	rfSPICommand.pData = pBuf;	
	rfSPICommand.Register = iRF_CMD_RD_RX_PLOAD;
	rfSPICommand.DataSize = TX_PLOAD_WIDTH;
	bool result = spi_read_buf(rfSPICommand);	
	return result;	
}

bool SE8R01_WritePayload(uint8_t* pBuf, bool ack)
{
	uint8_t status;	
	rfSPICommand.pResult = &status;	
	rfSPICommand.pData = pBuf;	
	rfSPICommand.Register = ack? iRF_CMD_WR_TX_PLOAD : iRF_CMD_W_TX_PAYLOAD_NOACK;
	rfSPICommand.DataSize = TX_PLOAD_WIDTH;
	bool result = spi_write_buf(rfSPICommand);		
	return result;	
}

bool SE8R01_FlushBuffers()
{
	uint8_t status;	
	rfSPICommand.pResult = &status;	
	rfSPICommand.pData = &status;	
	rfSPICommand.Register = iRF_CMD_FLUSH_RX;
	rfSPICommand.DataSize = 0;
	bool result = spi_rw_reg(rfSPICommand);
	rfSPICommand.Register = iRF_CMD_FLUSH_TX;
	return result && spi_rw_reg(rfSPICommand); 
}

extern uint8_t SE8R01_ReadStatus()
{
	uint8_t status;	
	rfSPICommand.pResult = &status;	
	rfSPICommand.pData = &status;	
	rfSPICommand.Register = iRF_BANK0_STATUS;
	rfSPICommand.DataSize = 0;
	bool result = spi_rw(rfSPICommand);
	if (result) return status;
	return 0;
}
