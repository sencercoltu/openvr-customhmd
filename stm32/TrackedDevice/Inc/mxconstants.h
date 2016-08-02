/**
  ******************************************************************************
  * File Name          : mxconstants.h
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MXCONSTANT_H
#define __MXCONSTANT_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define CTL_TRIGGER_Pin GPIO_PIN_1
#define CTL_TRIGGER_GPIO_Port GPIOA
#define CTL_AX_Pin GPIO_PIN_2
#define CTL_AX_GPIO_Port GPIOA
#define CTL_AY_Pin GPIO_PIN_3
#define CTL_AY_GPIO_Port GPIOA
#define IR_SENS_1_Pin GPIO_PIN_4
#define IR_SENS_1_GPIO_Port GPIOA
#define IR_SENS_2_Pin GPIO_PIN_5
#define IR_SENS_2_GPIO_Port GPIOA
#define IR_SENS_3_Pin GPIO_PIN_6
#define IR_SENS_3_GPIO_Port GPIOA
#define IR_SENS_4_Pin GPIO_PIN_7
#define IR_SENS_4_GPIO_Port GPIOA
#define CTL_B_1__IR_SENS_5_Pin GPIO_PIN_0
#define CTL_B_1__IR_SENS_5_GPIO_Port GPIOB
#define CTL_B_2__IR_SENS_6_Pin GPIO_PIN_1
#define CTL_B_2__IR_SENS_6_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_10
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_11
#define I2C_SDA_GPIO_Port GPIOB
#define CTL_VIBRATE_Pin GPIO_PIN_12
#define CTL_VIBRATE_GPIO_Port GPIOB
#define SPI_SCK_Pin GPIO_PIN_13
#define SPI_SCK_GPIO_Port GPIOB
#define SPI_MISO_Pin GPIO_PIN_14
#define SPI_MISO_GPIO_Port GPIOB
#define SPI_MOSI_Pin GPIO_PIN_15
#define SPI_MOSI_GPIO_Port GPIOB
#define SPI_RF_NSS_Pin GPIO_PIN_6
#define SPI_RF_NSS_GPIO_Port GPIOB
#define SPI_RF_CE_Pin GPIO_PIN_7
#define SPI_RF_CE_GPIO_Port GPIOB
#define CTL_MODE2_Pin GPIO_PIN_8
#define CTL_MODE2_GPIO_Port GPIOB
#define CTL_MODE1_Pin GPIO_PIN_9
#define CTL_MODE1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MXCONSTANT_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
