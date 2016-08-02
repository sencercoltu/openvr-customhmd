/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include "..\\..\\Common\\led.h"
#include "usbd_custom_hid_if.h"
//#include "..\\..\\Common\\SensorFusion.h"
//#include "..\\..\\Common\\gy8x.h"
//#include "..\\..\\Common\\se8r01.h"
#include "..\\..\\Common\\usb.h"
#include "..\\..\\Common\\nrf24l01.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

struct CommChannel
{
	uint8_t address[5];
	uint8_t remainingTransmits;
	uint32_t lastReceive;
	uint32_t nextTransmit;
	USBPacket lastCommand;
};

struct CommChannel Channels[5] = {0};

int32_t blinkDelay = 1000;
USBPacket USBBypassPacket = {0};
extern uint8_t USB_RX_Buffer[CUSTOM_HID_EPIN_SIZE];
USBPacket *pUSBCommandPacket = (USBPacket *) USB_RX_Buffer;
uint8_t rfStatus = 0;

#if CUSTOM_HID_EPOUT_SIZE != 32
#error HID out packet size not 32
#end
#elif CUSTOM_HID_EPIN_SIZE != 32
#error HID packet in size not 32
#end
#elif USB_CUSTOM_HID_DESC_SIZ != 34
#error HID packet size not 34
#end
#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
RF_InitTypeDef RF_InitStruct = {0};
unsigned char BaseStation_NRF24L01_Init (void)
{	
	RF_InitStruct.RF_Power_State=RF_Power_On;
	RF_InitStruct.RF_Config=RF_Config_IRQ_RX_Off|RF_Config_IRQ_TX_Off|RF_Confing_IRQ_Max_Rt_Off;
	RF_InitStruct.RF_CRC_Mode=RF_CRC8_On;
	RF_InitStruct.RF_Mode=RF_Mode_RX;
	RF_InitStruct.RF_Pipe_Auto_Ack=0;	
	RF_InitStruct.RF_Enable_Pipe=31;	
	RF_InitStruct.RF_Setup=RF_Setup_5_Byte_Adress;
	RF_InitStruct.RF_TX_Power=RF_TX_Power_High;
	RF_InitStruct.RF_Data_Rate=RF_Data_Rate_2Mbs;
	RF_InitStruct.RF_Channel=250;
	RF_InitStruct.RF_TX_Adress[0]=0x53;
	RF_InitStruct.RF_TX_Adress[1]=0x45;
	RF_InitStruct.RF_TX_Adress[2]=0x4E;
	RF_InitStruct.RF_TX_Adress[3]=0x43;
	RF_InitStruct.RF_TX_Adress[4]=0xC0;
	RF_InitStruct.RF_RX_Adress_Pipe0[0]=0x53;
	RF_InitStruct.RF_RX_Adress_Pipe0[1]=0x45;
	RF_InitStruct.RF_RX_Adress_Pipe0[2]=0x4E;
	RF_InitStruct.RF_RX_Adress_Pipe0[3]=0x43;
	RF_InitStruct.RF_RX_Adress_Pipe0[4]=0xC0;
	RF_InitStruct.RF_RX_Adress_Pipe1[0]=0x53;
	RF_InitStruct.RF_RX_Adress_Pipe1[1]=0x45;
	RF_InitStruct.RF_RX_Adress_Pipe1[2]=0x4E;
	RF_InitStruct.RF_RX_Adress_Pipe1[3]=0x43;
	RF_InitStruct.RF_RX_Adress_Pipe1[4]=0xC1;
	RF_InitStruct.RF_RX_Adress_Pipe2=0xC2;
	RF_InitStruct.RF_RX_Adress_Pipe3=0xB1;
	RF_InitStruct.RF_RX_Adress_Pipe4=0xB2;
	RF_InitStruct.RF_Payload_Size_Pipe0=32;
	RF_InitStruct.RF_Payload_Size_Pipe1=32;
	RF_InitStruct.RF_Payload_Size_Pipe2=32;
	RF_InitStruct.RF_Payload_Size_Pipe3=32;
	RF_InitStruct.RF_Payload_Size_Pipe4=32;
	RF_InitStruct.RF_Auto_Retransmit_Count=1;
	RF_InitStruct.RF_Auto_Retransmit_Delay=0;
	return RF_Init(&hspi1, &RF_InitStruct);
}
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
uint64_t HAL_GetMicros()
{
	uint64_t ticks = HAL_GetTick();
	uint64_t systick = (uint64_t)SysTick->VAL;
	return (ticks * 1000ull) + 1000ull - (systick / 72ul);
}

void HAL_MicroDelay(uint64_t delay)
{
	uint64_t tickstart = 0;	
	tickstart = HAL_GetMicros();
	while((HAL_GetMicros() - tickstart) < delay)
	{
	}
}


USBPacket SyncPacket = {0};
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == IR_SYNC_Pin)
	{
		SyncPacket.Command.Data.Sync.SyncTime = HAL_GetMicros();
		SetPacketCrc(&SyncPacket);
		USBD_CUSTOM_HID_SendReport_FS((uint8_t*)&SyncPacket, sizeof(USBPacket)); //forward incoming					
		LedToggle();
	}
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	uint16_t usbDescLen = 0;
	uint8_t *usbDesc = USBD_CUSTOM_HID.GetFSConfigDescriptor(&usbDescLen);
	usbDesc[33] = usbDesc[40] = 1;	//1 ms poll interval

	
	SyncPacket.Header.Type = BASESTATION_SOURCE | COMMAND_DATA;
	SyncPacket.Header.Sequence = 1;
	SyncPacket.Header.Crc8 = 0;
	SyncPacket.Command.Command = CMD_SYNC;
	SyncPacket.Command.Data.Sync.SyncTime = HAL_GetMicros();
	
	
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */
	BlinkRease(80);
	
	LedOff();		
	rfStatus = BaseStation_NRF24L01_Init();	
	if (rfStatus == 0x00 || rfStatus == 0xff)
		Error_Handler();
	LedOff();	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	bool readyToReceive = false;
	bool readyToSend = false;
	uint32_t now;
	
	srand(HAL_GetTick());
	for (uint8_t idx=0; idx<5; idx++)
	{
		memcpy(Channels[idx].address, RF_InitStruct.RF_RX_Adress_Pipe0, 5); //address of hmd
		Channels[idx].address[4] += idx;						
	}
	
	while (1)
	{
		now = HAL_GetTick();
		Blink(blinkDelay);
		blinkDelay = 1000;

		if (((pUSBCommandPacket->Header.Type & 0xF0) == COMMAND_DATA) && (pUSBCommandPacket->Command.Command != CMD_NONE))
		{		
			uint8_t channelIndex = pUSBCommandPacket->Header.Type & 0x0F; 			
			CommChannel *pChannel = &Channels[channelIndex];
			
			//only send if active more than 1 sec
			if (now - pChannel->lastReceive <= 1000)
			{			
				//got packet from usb, forward to trackeddevice
				memcpy(&pChannel->lastCommand, pUSBCommandPacket, sizeof(USBPacket));			
				//send same packet a few times
				pChannel->remainingTransmits = 2;			
				pChannel->nextTransmit = now;
			}
			memset(pUSBCommandPacket, 0, sizeof(USBPacket)); //clear incoming
		}
		
		if (!readyToReceive)
		{
			rfStatus = RF_FifoStatus(&hspi1);
			readyToReceive = (rfStatus & RF_RX_FIFO_EMPTY_Bit) == 0x00; //if not empty 
			readyToSend = ((rfStatus & RF_TX_FIFO_EMPTY_Bit) == RF_TX_FIFO_EMPTY_Bit); //if empty 
		}
				
		while (readyToReceive)
		{			
			readyToReceive = RF_ReceivePayload(&hspi1, (uint8_t*)&USBBypassPacket, sizeof(USBPacket)) == 0; //int cleared in function if fifo empty	
			if (CheckPacketCrc(&USBBypassPacket))
			{
				uint8_t channelIndex = USBBypassPacket.Header.Type & 0x0F; 
				Channels[channelIndex].lastReceive = now;
				blinkDelay = 100;
				USBD_CUSTOM_HID_SendReport_FS((uint8_t*)&USBBypassPacket, sizeof(USBPacket)); //forward incoming					
			}
		}

		if (readyToSend)
		{			
			for (uint8_t idx=0; idx<5; idx++)
			{
				CommChannel *pChannel = &Channels[idx];
				if (pChannel->remainingTransmits > 0 && now > pChannel->nextTransmit)
				{
					pChannel->nextTransmit = HAL_GetTick() + (rand() % 10); //max 10 ms for retransmit
					//switch to tx mode and send command to device			
					RF_TransmitMode(&hspi1, pChannel->address);
					RF_SendPayload(&hspi1, (uint8_t *)&pChannel->lastCommand, sizeof(USBPacket));
					do { rfStatus = RF_FifoStatus(&hspi1); } while ((rfStatus & RF_TX_FIFO_EMPTY_Bit) != RF_TX_FIFO_EMPTY_Bit); //wait for send complete
					pChannel->remainingTransmits--;
					if (!pChannel->remainingTransmits)					
						memset(&pChannel->lastCommand, 0, sizeof(USBPacket));					
				}
			}
			//back to rx mode
			RF_ReceiveMode(&hspi1, RF_InitStruct.RF_TX_Adress);
			readyToSend = false;			
		}
		

	}
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_RF_NSS_GPIO_Port, SPI_RF_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_RF_CE_GPIO_Port, SPI_RF_CE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_SYNC_Pin */
  GPIO_InitStruct.Pin = IR_SYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_SYNC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI_RF_NSS_Pin SPI_RF_CE_Pin */
  GPIO_InitStruct.Pin = SPI_RF_NSS_Pin|SPI_RF_CE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
	//while(1)
	//{
		BlinkRease(50, false);
		//BlinkRease(80, true);
	//}
  LedOff();
//  __HAL_RCC_SPI1_FORCE_RESET();
//  __HAL_RCC_SPI1_CLK_ENABLE();
//  __HAL_RCC_I2C2_FORCE_RESET();
//  __HAL_RCC_I2C2_CLK_ENABLE();
  NVIC_SystemReset();
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
