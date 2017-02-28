#ifndef _NRF24_H_
#define _NRF24_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "spilib.h"

extern void HAL_MicroDelay(uint64_t delay);
	
#define Config_Reg_Reset_Value	0x0E
#define Max_Data_Len 			32		//maximum Bytes in Payload, typ. 32
#define Max_Adress_Len 			5		//maximum Bytes in Data Adress, typ. 5
#define Init_Delay_us 			4000    //useconds wait for init chip delay, typ. 4000us
#define Payload_send_delay		2500	//maximum delay in cycles for data send ready in case of error

// NRF24l01 registers define
#define CONFIG_REG			0x00
#define EN_AA_REG			0x01
#define EN_RXADDR_REG		0x02
#define SETUP_AW_REG		0x03
#define	SETUP_RETR_REG		0x04
#define RF_CH_REG			0x05
#define RF_SETUP_REG		0x06
#define STATUS_REG			0x07
#define OBSERV_TX_REG		0x08
#define CD_REG				0x09
#define RX_ADDR_P0_REG		0x0A
#define RX_ADDR_P1_REG		0x0B
#define RX_ADDR_P2_REG		0x0C
#define RX_ADDR_P3_REG		0x0D
#define RX_ADDR_P4_REG		0x0E
#define RX_ADDR_P5_REG		0x0F
#define TX_ADDR_REG			0x10
#define RX_PW_P0_REG		0x11
#define RX_PW_P1_REG		0x12
#define RX_PW_P2_REG		0x13
#define RX_PW_P3_REG		0x14
#define RX_PW_P4_REG		0x15
#define RX_PW_P5_REG		0x16
#define FIFO_STATUS_REG		0x17

/* NRF24l01 some bits definition */
#define RF_Config_PWR_UP_Bit    (1<<1)
#define RF_RX_FIFO_EMPTY_Bit		(1<<0)
#define RF_TX_FIFO_EMPTY_Bit		(1<<4)
#define RF_RX_FIFO_FULL_Bit			(1<<1)
#define RF_TX_FIFO_FULL_Bit			(1<<5)
#define RF_TX_STATUS_FULL_Bit			(1<<0)

/* NRF24l01 commands define */
#define RF_SendPayload_CMD      0xA0
#define RF_Sleep_State_CMD      0x50
#define RF_Flush_TX_CMD         0xE1
#define RF_Flush_RX_CMD         0xE2

/* NRF24L01 error codes definition */
#define RF_SUCCESS																0x00
#define RF_DATA_SEND_NO_ACK_RECEIVED							0x00
#define RF_DATA_SEND_ACK_RECEIVED_OK							0x01
#define RF_ERROR_CHIP_NOT_RESPONDING 							0xFF
#define RF_NO_DATA_RECEIVED												0xFF
#define RF_NOT_READY	0xFF
											  
typedef struct
{
  unsigned char RF_Power_State;			//On/OFF Chip	
  unsigned char RF_Config;		  			//IRQ deffinition
  unsigned char RF_CRC_Mode;				//CRC mode
  unsigned char RF_Mode;		  			//TX or RX
  unsigned char RF_Pipe_Auto_Ack;			//0x01 EN_AA Enhanced ShockBurst
  unsigned char RF_Enable_Pipe;			//Enabling PIPEx
  unsigned char RF_Setup;					//3,4,5 bytes adress
  unsigned char RF_Channel;				//0...127 RF channel
  unsigned char RF_TX_Power;				//0,-6,-12,-18 dB RF Power for TX
  unsigned char RF_Data_Rate;				//1 or 2 Mbit/s
  unsigned char RF_RX_Adress_Pipe0[5];		//Adress for PIPE0
  unsigned char RF_RX_Adress_Pipe1[5];		//Adress for PIPE1
  unsigned char RF_RX_Adress_Pipe2;		//Adress for PIPE2
  unsigned char RF_RX_Adress_Pipe3;		//Adress for PIPE3
  unsigned char RF_RX_Adress_Pipe4;		//Adress for PIPE4
  unsigned char RF_RX_Adress_Pipe5;		//Adress for PIPE5
  unsigned char RF_TX_Adress[5];			//Adress for Transfer
  unsigned char RF_Payload_Size_Pipe0;		//0...32 bytes for PIPE0
  unsigned char RF_Payload_Size_Pipe1;		//0...32 bytes for PIPE1
  unsigned char RF_Payload_Size_Pipe2;		//0...32 bytes for PIPE2
  unsigned char RF_Payload_Size_Pipe3;		//0...32 bytes for PIPE3
  unsigned char RF_Payload_Size_Pipe4;		//0...32 bytes for PIPE4
  unsigned char RF_Payload_Size_Pipe5;		//0...32 bytes for PIPE5
  unsigned char RF_Auto_Retransmit_Delay;	//0...32 delay between retransmits
  unsigned char RF_Auto_Retransmit_Count;	//0...32 retrasmit counts
}RF_InitTypeDef;

//RF_Power_State
#define RF_Power_On				((unsigned char)0x02)				  
#define RF_Power_Off			((unsigned char)0x00)
//RF_Config
#define RF_Config_IRQ_RX_On		((unsigned char)0x00)
#define RF_Config_IRQ_RX_Off	((unsigned char)0x40)

#define RF_Config_IRQ_TX_On		((unsigned char)0x00)
#define RF_Config_IRQ_TX_Off	((unsigned char)0x20)

#define RF_Confing_IRQ_Max_Rt_On  ((unsigned char)0x00)
#define RF_Confing_IRQ_Max_Rt_Off ((unsigned char)0x10)
//RF_CRC_Mode
#define RF_CRC_Off				((unsigned char)0x00)
#define RF_CRC8_On				((unsigned char)0x08)
#define RF_CRC16_On				((unsigned char)0x0C)
//RF_Mode
#define RF_Mode_TX				((unsigned char)0x00)
#define RF_Mode_RX				((unsigned char)0x01)
//RF_Pipe_Auto_Ack
#define RF_Pipe_All_Ack_Disable ((unsigned char)0x00)
#define RF_Pipe_All_Ack_Enable	((unsigned char)0x3F)
#define RF_Pipe0_Ack_Enable		((unsigned char)0x01)
#define RF_Pipe1_Ack_Enable		((unsigned char)0x02)
#define RF_Pipe2_Ack_Enable		((unsigned char)0x04)
#define RF_Pipe3_Ack_Enable		((unsigned char)0x08)
#define RF_Pipe4_Ack_Enable		((unsigned char)0x10)
#define RF_Pipe5_Ack_Enable		((unsigned char)0x20)
//RF_Enable_Pipe
#define RF_Pipe_All_Enable		((unsigned char)0x3F)
#define RF_Pipe_All_Disable		((unsigned char)0x00)
#define RF_Pipe0_Enable			((unsigned char)0x01)
#define RF_Pipe1_Enable			((unsigned char)0x02)
#define RF_Pipe2_Enable			((unsigned char)0x04)
#define RF_Pipe3_Enable			((unsigned char)0x08)
#define RF_Pipe4_Enable			((unsigned char)0x10)
#define RF_Pipe5_Enable			((unsigned char)0x20)
//RF_Setup
#define RF_Setup_3_Byte_Adress  ((unsigned char)0x01) 
#define RF_Setup_4_Byte_Adress  ((unsigned char)0x02) 
#define RF_Setup_5_Byte_Adress  ((unsigned char)0x03)
//RF_TX_Power
#define RF_TX_Power_High		((unsigned char)0x06)
#define RF_TX_Power_Medium		((unsigned char)0x04)
#define RF_TX_Power_Low			((unsigned char)0x02)
#define RF_TX_Power_Lowest		((unsigned char)0x00)
//RF_Data_Rate
#define RF_Data_Rate_1Mbs		((unsigned char)0x00)
#define RF_Data_Rate_2Mbs		((unsigned char)0x08) 

#define RF_Power_State(STATE)	(((STATE) == RF_Power_On)||\((STATE) == RF_Power_Off))

#define RF_Config(CONFIG)		(((CONFIG) == RF_Config_IRQ_RX_On)||\((CONFIG) == RF_Config_IRQ_RX_Off) || \
								 ((CONFIG) == RF_Config_IRQ_TX_On)||\((CONFIG) == RF_Config_IRQ_TX_Off) || \
								 ((CONFIG) == RF_Confing_IRQ_Max_Rt_On)||\((CONFIG) == RF_Confing_IRQ_Max_Rt_Off))

#define RF_CRC_Mode(CRC)		(((CRC) == RF_CRC_Off)||\((CRC) == RF_CRC8_On)||\((CRC) == RF_CRC16_On))

#define RF_Mode(MODE)			(((MODE) == RF_Mode_TX)||\((MODE) == RF_Mode_RX))

#define RF_Pipe_Auto_Ack(ACK)	(((ACK) == RF_Pipe_All_Ack_Disable)||\((ACK) == RF_Pipe_All_Ack_Enable)	|| \
								 ((ACK) == RF_Pipe0_Ack_Enable)||\((ACK) == RF_Pipe1_Ack_Enable)		|| \
								 ((ACK) == RF_Pipe2_Ack_Enable)||\((ACK) == RF_Pipe3_Ack_Enable)		|| \
								 ((ACK) == RF_Pipe4_Ack_Enable)||\((ACK) == RF_Pipe5_Ack_Enable))

#define RF_Enable_Pipe(ENABLE_PIPE)		(((ENABLE_PIPE) == RF_Pipe_All_Enable)||\((ENABLE_PIPE) == RF_Pipe_All_Disable)	|| \
								 		((ENABLE_PIPE) == RF_Pipe0_Enable)||\((ENABLE_PIPE) == RF_Pipe1_Enable)		|| \
								 		((ENABLE_PIPE) == RF_Pipe2_Enable)||\((ENABLE_PIPE) == RF_Pipe3_Enable)		|| \
								 		((ENABLE_PIPE) == RF_Pipe4_Enable)||\((ENABLE_PIPE) == RF_Pipe5_Enable))   

#define RF_Setup(ADRS)		(((ADRS) == RF_Setup_3_Byte_Adress)||\((ADRS) == RF_Setup_4_Byte_Adress)||\((ADRS) == RF_Setup_5_Byte_Adress))

#define RF_TX_Power(POWER)	(((POWER) == RF_TX_Power_High)||\((POWER) == RF_TX_Power_Medium) || \
							((POWER) == RF_TX_Power_Low)||\ ((POWER) == RF_TX_Power_Lowest))

#define RF_Data_Rate(RATE)	(((RATE) == RF_Data_Rate_1Mbs)||\((RATE) == RF_Data_Rate_2Mbs))

#define RF_RX_DR_IRQ_CLEAR			0x40
#define RF_TX_DS_IRQ_CLEAR			0x20
#define RF_MAX_RT_IRQ_CLEAR			0x10

uint8_t RF_Init(SPI_HandleTypeDef* SPIx, RF_InitTypeDef* RF_InitStruct);

uint8_t RF_DisableTransmit(SPI_HandleTypeDef* SPIx);
uint8_t RF_EnableTransmit(SPI_HandleTypeDef* SPIx);

uint8_t RF_SendPayload(SPI_HandleTypeDef* SPIx, uint8_t* data, uint8_t DataLen);
uint8_t RF_ReceivePayload(SPI_HandleTypeDef* SPIx, uint8_t* Data, uint8_t Dala_Len);

uint8_t RF_ClearRxInt(SPI_HandleTypeDef* SPIx);
uint8_t RF_ClearTxInt(SPI_HandleTypeDef* SPIx);

uint8_t RF_Carrier_Detect(SPI_HandleTypeDef* SPIx);
uint8_t RF_Count_Lost_Packets(SPI_HandleTypeDef* SPIx);
uint8_t RF_Count_Resend_Packets(SPI_HandleTypeDef* SPIx);

uint8_t RF_Status(SPI_HandleTypeDef* SPIx);
uint8_t RF_FifoStatus(SPI_HandleTypeDef* SPIx);

uint8_t RF_TransmitMode(SPI_HandleTypeDef* SPIx, uint8_t *address);
uint8_t RF_ReceiveMode(SPI_HandleTypeDef* SPIx, uint8_t *address);

#ifdef __cplusplus
}
#endif

#endif //_NRF24_H_
