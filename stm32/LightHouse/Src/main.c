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

/* USER CODE BEGIN Includes */
#include "..\..\Common\led.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

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

volatile int step = 0;
#define SYNC_MARK 0xD3
#define VERT 0x21
#define HORZ 0x41
#define REVERSE 0x08
#define PULSE_WIDTH 600

volatile bool selfSync = false; 


uint16_t SyncData = 0;
uint8_t SyncBits = 0;
uint64_t LastSyncBitTime = 0;
uint8_t LastSyncBit = 0;
uint32_t SyncElapsedTime = 0;
uint8_t SyncElapsedPulses = 0;
uint8_t SyncCurrBit = 0;
volatile uint8_t SyncCommand = 0;

uint32_t NextTurn = 0;

uint16_t SyncDebug[16] = {0};
uint8_t SyncDebugIdx = 0;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (selfSync) return; //prevent self sync signal process
	if (GPIO_Pin == SYNC_RECV_Pin)
	{	
		uint64_t now = HAL_GetMicros();
		//interrupt on up and down edges
		SyncCurrBit = (HAL_GPIO_ReadPin(SYNC_RECV_GPIO_Port, SYNC_RECV_Pin) == GPIO_PIN_RESET)? 1 : 0;
		if (SyncCurrBit)
		{			
			SyncElapsedTime = (uint32_t)(now - LastSyncBitTime);			
			SyncElapsedPulses = (uint8_t)(((float)SyncElapsedTime / (float)PULSE_WIDTH) + 0.5f) ;						
			if (SyncElapsedPulses == 1)
			{
				SyncData = (SyncData << 1);	// 0
				SyncBits++;
			}
			else if (SyncElapsedPulses == 3)				
			{
				SyncData = (SyncData << 1) | 1;	// 1			
				SyncBits++;
			}
			else
			{
				SyncBits = 0;
				SyncData = 0;
				SyncCommand = 0;
				//LedOff();
			}
			
			if (SyncBits == 16)
			{
				if (((SyncData >> 8) & SYNC_MARK) == SYNC_MARK)
				{
					SyncCommand = (uint8_t)(SyncData & 0xff);
					SyncDebug[SyncDebugIdx] = SyncData;
					SyncDebugIdx++;
					if (SyncDebugIdx == 16) SyncDebugIdx = 0;
				}
				else
				{
					SyncCommand = 0;
				}
				SyncBits = 0;
				SyncData = 0;
				//LedOn();
			}
		}
		LastSyncBitTime = now;
	}
}

uint32_t delay = 0;
int pwm = 0;



void SendIREndMark()
{
	TIM1->CCR1 = 38; //set %50 duty
	HAL_MicroDelay(PULSE_WIDTH);
	TIM1->CCR1 = 0; //set %0 duty
	HAL_MicroDelay(PULSE_WIDTH);		
	selfSync = false;	
}

uint32_t SendIRByte(uint8_t data)
{	
	uint32_t pulses = 0;
	selfSync = true;
	for (int i=0; i<8; i++)
	{		
		if ((data & 0x80) == 0x80)			
		{
			TIM1->CCR1 = 38; //set %50 duty
			HAL_MicroDelay(PULSE_WIDTH);
			TIM1->CCR1 = 0; //set %0 duty
			HAL_MicroDelay(PULSE_WIDTH*3);
			pulses += 4;
		}
		else
		{
			TIM1->CCR1 = 38; //set %50 duty
			HAL_MicroDelay(PULSE_WIDTH);
			TIM1->CCR1 = 0; //set %0 duty
			HAL_MicroDelay(PULSE_WIDTH);
			pulses += 2;
		}
		data = data << 1;				
	}
	return (32 - pulses) * PULSE_WIDTH;
}

void SetLedDuty(float x)
{	
	
	
	if (x < 0) x = 0.0f;
	if (x > 100) x = 100.0f;
	TIM3->CCR4 = (uint32_t)(((float)(TIM3->ARR) / 100.0f) * x);
	
}
#define SERVO_90 130
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	//Determine the desired PWM resolution (for example: 100 steps, 200 steps, 1000…)
	//Determine the desired PWM frequency (for example: 1kHz, 10kHz, 40kHz, …)
	//Set the Counter Period at the same value as the resolution
	//Set the Prescaler = 48000000/(PWM frequency*PWM resolution) – 1 (thanks to Ruben and Avinash to spot out the mistake)
	//Set the Pulse equal to the desired Pulse width where the value varies from 0 to Counter Period.	
	
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();

  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);		
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);		
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);		
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	uint32_t extraDelay;
	uint8_t direction = 0;
	uint32_t angle = 2000;
	
	uint8_t lightHouseId = (GPIO_PIN_SET == HAL_GPIO_ReadPin(LH_ID_GPIO_Port, LH_ID_Pin)? 0x10 : 0x00); 
	//uint32_t lastCommandTime = HAL_GetTick();
	
	if (!lightHouseId) SyncCommand = HORZ;
	TIM1->CCR1 = 0; //set %0 duty
	
	while (true)
	{		
		uint32_t now = HAL_GetTick();
		if (now > NextTurn) SyncCommand = HORZ;			
		if (SyncCommand == 0) continue;
		if ((SyncCommand & HORZ) == HORZ)
		{				
			//lastCommandTime = now;
			if (angle == 2000)
				angle = 1000;
			else
				angle = 2000;			
			direction = (angle == 1000)? 0: REVERSE;				
			
			HAL_Delay(SERVO_90); // wait until remote finish
			LedOn();
			
			extraDelay = SendIRByte(SYNC_MARK); //send sync magic
			extraDelay += SendIRByte(VERT | lightHouseId | direction);	//send sync info
			HAL_GPIO_WritePin(LASER_VERT_GPIO_Port, LASER_VERT_Pin, GPIO_PIN_SET); //turn on vert laser
			SendIREndMark();
			//21 ms total  = 34 * 600 = 21400microsec
			if (extraDelay) HAL_MicroDelay(extraDelay);		
			TIM2->CCR1 = angle; //move vert to 0
			HAL_Delay(SERVO_90); // wait until finish
			HAL_GPIO_WritePin(LASER_VERT_GPIO_Port, LASER_VERT_Pin, GPIO_PIN_RESET); //turn off vert laser		
			
			
			extraDelay = SendIRByte(SYNC_MARK); //send sync magic
			extraDelay += SendIRByte(HORZ | lightHouseId | direction);	//send sync info
			HAL_GPIO_WritePin(LASER_HORZ_GPIO_Port, LASER_HORZ_Pin, GPIO_PIN_SET); //turn on horz laser
			SendIREndMark();
			if (extraDelay) HAL_MicroDelay(extraDelay);
			TIM2->CCR2 = angle; 
			HAL_Delay(SERVO_90); //wait fin
			HAL_GPIO_WritePin(LASER_HORZ_GPIO_Port, LASER_HORZ_Pin, GPIO_PIN_RESET); //turn off horz laser
			
			LedOff();	
			NextTurn = HAL_GetTick() + ((SERVO_90 + 21) * 2) + 10;
		}
		SyncCommand = 0;
		
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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 24;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 75;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 38;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim1);

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 10000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim2);

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
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LASER_VERT_Pin|LASER_HORZ_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LASER_VERT_Pin LASER_HORZ_Pin */
  GPIO_InitStruct.Pin = LASER_VERT_Pin|LASER_HORZ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SYNC_RECV_Pin */
  GPIO_InitStruct.Pin = SYNC_RECV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SYNC_RECV_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LH_ID_Pin */
  GPIO_InitStruct.Pin = LH_ID_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(LH_ID_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

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
