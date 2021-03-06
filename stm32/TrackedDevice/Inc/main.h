/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define IR_SENS4_Pin GPIO_PIN_14
#define IR_SENS4_GPIO_Port GPIOC
#define IR_SENS5_Pin GPIO_PIN_15
#define IR_SENS5_GPIO_Port GPIOC
#define CTL_XTRA_Pin GPIO_PIN_0
#define CTL_XTRA_GPIO_Port GPIOA
#define CTL_TRIGGER_Pin GPIO_PIN_1
#define CTL_TRIGGER_GPIO_Port GPIOA
#define CTL_BTN7_Pin GPIO_PIN_2
#define CTL_BTN7_GPIO_Port GPIOA
#define CTL_BTN0_Pin GPIO_PIN_3
#define CTL_BTN0_GPIO_Port GPIOA
#define CTL_BTN1_Pin GPIO_PIN_4
#define CTL_BTN1_GPIO_Port GPIOA
#define CTL_BTN2_Pin GPIO_PIN_5
#define CTL_BTN2_GPIO_Port GPIOA
#define CTL_BTN3_Pin GPIO_PIN_6
#define CTL_BTN3_GPIO_Port GPIOA
#define CTL_BTN4_Pin GPIO_PIN_7
#define CTL_BTN4_GPIO_Port GPIOA
#define CTL_BTN5_Pin GPIO_PIN_0
#define CTL_BTN5_GPIO_Port GPIOB
#define CTL_BTN6_Pin GPIO_PIN_1
#define CTL_BTN6_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_10
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_11
#define I2C_SDA_GPIO_Port GPIOB
#define IR_SYNC_Pin GPIO_PIN_12
#define IR_SYNC_GPIO_Port GPIOB
#define SPI_SCK_Pin GPIO_PIN_13
#define SPI_SCK_GPIO_Port GPIOB
#define SPI_MISO_Pin GPIO_PIN_14
#define SPI_MISO_GPIO_Port GPIOB
#define SPI_MOSI_Pin GPIO_PIN_15
#define SPI_MOSI_GPIO_Port GPIOB
#define IR_SENS0_Pin GPIO_PIN_8
#define IR_SENS0_GPIO_Port GPIOA
#define IR_SENS1_Pin GPIO_PIN_9
#define IR_SENS1_GPIO_Port GPIOA
#define IR_SENS2_Pin GPIO_PIN_10
#define IR_SENS2_GPIO_Port GPIOA
#define SPI_RF_NSS_Pin GPIO_PIN_6
#define SPI_RF_NSS_GPIO_Port GPIOB
#define SPI_RF_CE_Pin GPIO_PIN_7
#define SPI_RF_CE_GPIO_Port GPIOB
#define CTL_VIBRATE1_Pin GPIO_PIN_8
#define CTL_VIBRATE1_GPIO_Port GPIOB
#define CTL_VIBRATE0_Pin GPIO_PIN_9
#define CTL_VIBRATE0_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
