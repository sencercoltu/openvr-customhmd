/* VERSION 2.04 */
/*  20.12.2012  */
/* STM32 Version */

#include "nrf24l01.h"

unsigned char RF_Send_Cmd(SPI_HandleTypeDef* SPIx, unsigned char adrs, unsigned char cmd);
unsigned char RF_Read_Cmd(SPI_HandleTypeDef* SPIx, unsigned char adrs);
unsigned char RF_Send_Adrs(SPI_HandleTypeDef* SPIx, unsigned char adrs, unsigned char cmd[5]);
unsigned char RF_IRQ_CLEAR (SPI_HandleTypeDef* SPIx, unsigned char CMD);
unsigned char RF_Flush (SPI_HandleTypeDef* SPIx, unsigned char CMD);

void RF_CE_HIGH();
void RF_CE_LOW();
void RF_NSS_HIGH();
void RF_NSS_LOW();

extern void HAL_MicroDelay(uint64_t delay);

uint8_t RF_TransmitMode(SPI_HandleTypeDef* SPIx, uint8_t *address)
{
	uint8_t tmp = RF_Read_Cmd(SPIx, CONFIG_REG);
	if ((tmp & ~RF_Mode_RX) == ~RF_Mode_RX) return tmp;
	RF_CE_LOW();																
	tmp &= ~RF_Mode_RX;
	tmp = RF_Send_Cmd(SPIx, CONFIG_REG, tmp);
	if (address) 
		tmp = RF_Send_Adrs(SPIx, TX_ADDR_REG, address);		
	//flush
	//RF_Flush(SPIx, RF_Flush_TX_CMD);
	RF_CE_HIGH();
	HAL_Delay(1);
	return tmp;
}

uint8_t RF_ReceiveMode(SPI_HandleTypeDef* SPIx, uint8_t *address)
{
	uint8_t tmp = RF_Read_Cmd(SPIx, CONFIG_REG);															
	if ((tmp & RF_Mode_RX) == RF_Mode_RX) return tmp;
	RF_CE_LOW();	
	tmp |= RF_Mode_RX;	
	tmp = RF_Send_Cmd(SPIx, CONFIG_REG, tmp);
	if (address) 
		tmp = RF_Send_Adrs(SPIx, TX_ADDR_REG, address);
	//flush
	//RF_Flush(SPIx, RF_Flush_TX_CMD);	
	RF_CE_HIGH();
	HAL_Delay(1);
	return tmp;
}

unsigned char RF_Init(SPI_HandleTypeDef* SPIx, RF_InitTypeDef* RF_InitStruct)		   //if fast<>0 using short programming only for change RF_Mode RX/TX
{	
	RF_CE_LOW();	
    unsigned char tmp=RF_Send_Cmd(SPIx, CONFIG_REG, RF_InitStruct->RF_Config|RF_InitStruct->RF_Power_State|RF_InitStruct->RF_CRC_Mode|RF_InitStruct->RF_Mode);															
	tmp = RF_Send_Cmd(SPIx, EN_AA_REG,RF_InitStruct->RF_Pipe_Auto_Ack);										//EN_AA 		REG
	tmp = RF_Send_Cmd(SPIx, EN_RXADDR_REG,RF_InitStruct->RF_Enable_Pipe);									//EX_RXADDR		REG
	tmp = RF_Send_Cmd(SPIx, SETUP_AW_REG,RF_InitStruct->RF_Setup);											//SETUP_AW		REG
	tmp = RF_Send_Cmd(SPIx, SETUP_RETR_REG,(RF_InitStruct->RF_Auto_Retransmit_Count&0x0F)|(((RF_InitStruct->RF_Auto_Retransmit_Delay)&0x0F)<<4));
	tmp = RF_Send_Cmd(SPIx, RF_CH_REG,(RF_InitStruct->RF_Channel&0x7F));									//RF Chanel		REG
	tmp = RF_Send_Cmd(SPIx, RF_SETUP_REG,(0x01|RF_InitStruct->RF_TX_Power|RF_InitStruct->RF_Data_Rate));	//RF_SETUP		REG
		 
    if (RF_InitStruct->RF_RX_Adress_Pipe0[0])
		tmp = RF_Send_Adrs(SPIx, RX_ADDR_P0_REG,RF_InitStruct->RF_RX_Adress_Pipe0);								//RX adress for PIPE0
	if (RF_InitStruct->RF_RX_Adress_Pipe1[0])
		tmp = RF_Send_Adrs(SPIx, RX_ADDR_P1_REG,RF_InitStruct->RF_RX_Adress_Pipe1);								//RX adress for PIPE1
	if (RF_InitStruct->RF_RX_Adress_Pipe2)
		tmp = RF_Send_Cmd(SPIx, RX_ADDR_P2_REG,RF_InitStruct->RF_RX_Adress_Pipe2);								//RX adress for PIPE2 (only LSB) 
	if (RF_InitStruct->RF_RX_Adress_Pipe3)
		tmp = RF_Send_Cmd(SPIx, RX_ADDR_P3_REG,RF_InitStruct->RF_RX_Adress_Pipe3);								//RX adress for PIPE3 (only LSB)
	if (RF_InitStruct->RF_RX_Adress_Pipe4)
		tmp = RF_Send_Cmd(SPIx, RX_ADDR_P4_REG,RF_InitStruct->RF_RX_Adress_Pipe4);								//RX adress for PIPE4 (only LSB)
	if (RF_InitStruct->RF_RX_Adress_Pipe5)
		tmp = RF_Send_Cmd(SPIx, RX_ADDR_P5_REG,RF_InitStruct->RF_RX_Adress_Pipe5);								//RX adress for PIPE5 (only LSB)
    if (RF_InitStruct->RF_TX_Adress[0])
		tmp = RF_Send_Adrs(SPIx, TX_ADDR_REG,RF_InitStruct->RF_TX_Adress);
    if (RF_InitStruct->RF_Payload_Size_Pipe0)
		tmp = RF_Send_Cmd(SPIx, RX_PW_P0_REG,RF_InitStruct->RF_Payload_Size_Pipe0);
    if (RF_InitStruct->RF_Payload_Size_Pipe1)
		tmp = RF_Send_Cmd(SPIx, RX_PW_P1_REG,RF_InitStruct->RF_Payload_Size_Pipe1);
    if (RF_InitStruct->RF_Payload_Size_Pipe2)
		tmp = RF_Send_Cmd(SPIx, RX_PW_P2_REG,RF_InitStruct->RF_Payload_Size_Pipe2);
    if (RF_InitStruct->RF_Payload_Size_Pipe3)
		tmp = RF_Send_Cmd(SPIx, RX_PW_P3_REG,RF_InitStruct->RF_Payload_Size_Pipe3);
    if (RF_InitStruct->RF_Payload_Size_Pipe4)
		tmp = RF_Send_Cmd(SPIx, RX_PW_P4_REG,RF_InitStruct->RF_Payload_Size_Pipe4);
    if (RF_InitStruct->RF_Payload_Size_Pipe5)
		tmp = RF_Send_Cmd(SPIx, RX_PW_P5_REG,RF_InitStruct->RF_Payload_Size_Pipe5);
	
	RF_IRQ_CLEAR(SPIx, RF_RX_DR_IRQ_CLEAR);
	RF_Flush(SPIx, RF_Flush_RX_CMD);
	RF_IRQ_CLEAR(SPIx, RF_MAX_RT_IRQ_CLEAR|RF_TX_DS_IRQ_CLEAR);
	RF_Flush(SPIx, RF_Flush_TX_CMD);
	
	tmp=RF_Send_Cmd(SPIx, CONFIG_REG, RF_InitStruct->RF_Config|RF_Power_Off|RF_InitStruct->RF_CRC_Mode|RF_InitStruct->RF_Mode);
	HAL_Delay(10);
	tmp=RF_Send_Cmd(SPIx, CONFIG_REG, RF_InitStruct->RF_Config|RF_Power_On|RF_InitStruct->RF_CRC_Mode|RF_InitStruct->RF_Mode);

	RF_CE_HIGH();

	return tmp;			 
}

unsigned char RF_SendPayload(SPI_HandleTypeDef* SPIx, unsigned char * data, unsigned char DataLen)
{
  RF_CE_LOW();
  RF_NSS_LOW();
  SPI_SendByte(SPIx, RF_SendPayload_CMD);
  while(DataLen) 
  {
    SPI_SendByte(SPIx, *data++);
    DataLen--;
  }
  RF_NSS_HIGH();
  RF_CE_HIGH();
  return RF_SUCCESS;
}


unsigned char RF_ReceivePayload(SPI_HandleTypeDef* SPIx, unsigned char * Data, unsigned char Data_Len)					  			//returns 0xFF if no data received. Data_Len = num of bytes to receive
{																																							//returns PIPE num if data received succesfully		
	unsigned char temp;			
	//RF_CE_LOW();
	RF_NSS_LOW();
	temp = (SPI_ReadByte(SPIx, 0x61)&0x0E)>>1;					//read status reg and send R_RX_PAYLOAD Command															  	
	while(Data_Len)																			//read DATA from FIFO
	{
		*Data++=SPI_ReadByte(SPIx, 0xFF);
		Data_Len--;
	}
	RF_NSS_HIGH();
	
	temp = RF_Read_Cmd(SPIx, FIFO_STATUS_REG) & RF_RX_FIFO_EMPTY_Bit;	//check available data in RX FIFO, set if empty
	if (temp == 1)
		RF_IRQ_CLEAR(SPIx, RF_RX_DR_IRQ_CLEAR);	//if NO DATA in RX FIFO, clear IRQ
	//RF_CE_HIGH();
	return temp;
}

unsigned char RF_Send_Cmd(SPI_HandleTypeDef* SPIx, unsigned char adrs, unsigned char cmd)							//write data to register procedure
{	
	unsigned char temp=0;
	if ((adrs<0x0A)|(cmd!=0))
	{
		RF_NSS_LOW();
		temp=SPI_ReadByte(SPIx,(0x1F&adrs)|(1<<5));
		SPI_SendByte(SPIx,cmd);
		RF_NSS_HIGH();
	}
	return temp;
}

unsigned char RF_Read_Cmd(SPI_HandleTypeDef* SPIx, unsigned char adrs)										//read data from register
{		
	RF_NSS_LOW();
	SPI_SendByte(SPIx,0x1F&adrs);
	unsigned char temp = SPI_ReadByte(SPIx,0xFF);
	RF_NSS_HIGH();
	return temp;
}

unsigned char RF_Send_Adrs(SPI_HandleTypeDef* SPIx, unsigned char adrs, unsigned char cmd[Max_Adress_Len])
{	
	unsigned char temp = 0;
	if (cmd[0]!=0)
	{
		RF_NSS_LOW();
		SPI_SendByte(SPIx,adrs|(1<<5));
		for (temp=0;temp!=Max_Adress_Len;temp++) SPI_SendByte(SPIx,cmd[temp]);	
		RF_NSS_HIGH();
	}
	return temp;	
}

unsigned char RF_Carrier_Detect(SPI_HandleTypeDef* SPIx)										//returns 1 if Carrier Detected on current channel
{
	return (RF_Read_Cmd(SPIx, CD_REG)&0x01);
}

unsigned char RF_Count_Lost_Packets(SPI_HandleTypeDef* SPIx)									//returns num of Lost Packatets 
{
	return (((RF_Read_Cmd(SPIx, OBSERV_TX_REG))&0xF0)>>4);
}

unsigned char RF_Count_Resend_Packets(SPI_HandleTypeDef* SPIx)								//returns nut of Resend Packets
{
	return ((RF_Read_Cmd(SPIx, OBSERV_TX_REG))&0x0F);
}

unsigned char RF_IRQ_CLEAR (SPI_HandleTypeDef* SPIx, unsigned char CMD)											//Clears IRQ
{
	return RF_Send_Cmd(SPIx, STATUS_REG, CMD);
}
	

unsigned char RF_ClearRxInt(SPI_HandleTypeDef* SPIx)
{
	return RF_IRQ_CLEAR(SPIx, RF_RX_DR_IRQ_CLEAR);
}

unsigned char RF_ClearTxInt(SPI_HandleTypeDef* SPIx)
{
	return RF_IRQ_CLEAR(SPIx, RF_MAX_RT_IRQ_CLEAR|RF_TX_DS_IRQ_CLEAR);
}
	
	
unsigned char RF_Flush(SPI_HandleTypeDef* SPIx, unsigned char CMD)
{
	RF_NSS_LOW();
	unsigned char temp = SPI_SendByte(SPIx, CMD);
	RF_NSS_HIGH();
	return temp;
}


void RF_CE_HIGH()
{
	HAL_GPIO_WritePin(SPI_RF_CE_GPIO_Port, SPI_RF_CE_Pin, GPIO_PIN_SET); //PIN_OFF(RF_CSN_PIN);	
}

void RF_CE_LOW()
{
	HAL_GPIO_WritePin(SPI_RF_CE_GPIO_Port, SPI_RF_CE_Pin, GPIO_PIN_RESET); //PIN_OFF(RF_CSN_PIN);
}

void RF_NSS_HIGH()
{
	HAL_GPIO_WritePin(SPI_RF_NSS_GPIO_Port, SPI_RF_NSS_Pin, GPIO_PIN_SET); //PIN_OFF(RF_CSN_PIN);	
}

void RF_NSS_LOW()
{
	HAL_GPIO_WritePin(SPI_RF_NSS_GPIO_Port, SPI_RF_NSS_Pin, GPIO_PIN_RESET); //PIN_OFF(RF_CSN_PIN);
	HAL_MicroDelay(10);
}

unsigned char RF_Status(SPI_HandleTypeDef* SPIx)
{
	return RF_Read_Cmd(SPIx, STATUS_REG);
}

unsigned char RF_FifoStatus(SPI_HandleTypeDef* SPIx)
{
	return RF_Read_Cmd(SPIx, FIFO_STATUS_REG);
}
