/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#undef UNUSED
#define UNUSED(x) ;

#include <stdlib.h>
#include <string.h> //for memcpy and memcmp

#include "..\\..\\Common\\led.h"
#include "..\\..\\Common\\SensorFusion.h"
#include "..\\..\\Common\\gy8x.h"
#include "..\\..\\Common\\nrf24l01.h"
#include "usbd_custom_hid_if.h"
#include "..\\..\\Common\\usb.h"
#include "..\\..\\Common\\eeprom_flash.h"

#if CUSTOM_HID_EPOUT_SIZE != 32
#error HID out packet size not 32
//#end
#elif CUSTOM_HID_EPIN_SIZE != 32
#error HID packet in size not 32
//#end
#elif USB_CUSTOM_HID_DESC_SIZ != 34
#error HID packet size not 34
//#end
#endif

#define THIS_DEVICE (HMD_SOURCE)
//#define THIS_DEVICE LEFTCTL_SOURCE
#define IS_CONTROLLER (THIS_DEVICE == LEFTCTL_SOURCE || RIGHT_DEVICE == LEFTCTL_SOURCE)
#define IS_HMD (THIS_DEVICE == HMD_SOURCE)
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
enum RawModes
{
	RawMode_Off = 0,
	RawMode_Raw = 1,
	RawMode_Filtered = 2,
	RawMode_Compensated = 3,
	RawMode_DRVectors = 4
};

#if IS_HMD
struct CommChannel
{
	uint8_t remainingTransmits;
	uint32_t lastReceive;
	uint32_t nextTransmit;
	uint16_t lastInSequence;
	uint16_t lastOutSequence;
	USBPacket lastCommand;
};

struct CommChannel Channels[MAX_SOURCE] = { 0 };
uint16_t ChannelStateFlags = 0;
#endif //IS_HMD

SensorData tSensorData;
int32_t blinkDelay = 1000;
extern uint8_t USB_RX_Buffer[CUSTOM_HID_EPIN_SIZE];
extern USBD_HandleTypeDef hUsbDeviceFS;
USBPacket *pFromUSBPacket = (USBPacket *)USB_RX_Buffer;
USBPacket FromDevicePacket = { 0 };
USBPacket FromRFPacket = { 0 };
USBPositionData position = { 0 };

uint8_t rfStatus = 0;
bool sensorStatus = false;
RawModes feedRawMode = RawMode_Off;
uint8_t readyToSend = 0;
uint8_t readyToReceive = 0;
Quaternion orientQuat;
Quaternion gravQuat;
Quaternion positionQuat;
//Quaternion gravQuat(0, 0, 0, 1);
//RF_ReceiveMode(&hspi2, NULL); //default to receive mode			
uint32_t lastCommandSequence = 0;

#if IS_CONTROLLER
uint32_t vibrationStopTime0 = 0;
uint32_t vibrationStopTime1 = 0;
uint32_t LastADCChange = 0;
uint32_t ADC_Values[4] = { 0 };
float ADC_Value[2] = { 0 };
uint32_t ADC_Cache[2] = { 0 };
#endif //_IS_CONTROLLER

uint64_t lastButtonRefresh = 0;
volatile uint32_t LastDigitalChange = 0;
uint16_t DigitalValues = 0;
uint16_t DigitalCache = 0;
//float AccelVector[3] = {0};
float GravityVector[3] = { 0 };
float CompAccelVector[3] = { 0 };
float VelocityVector[3] = { 0 };
//float ComplimentaryAccelVector[3] = {0};
//float ComplimentaryVelocityVector[3] = {0};

uint64_t lastRotationUpdate = 0;
uint64_t now = 0;
uint32_t ticks = 0;
uint32_t nextSend = 0;

uint32_t sndCounter = 0;
uint32_t rcvCounter = 0;

EepromData eepromData = { 0 };
bool isCalibrated = false;
bool isPositionReady = false;

int extraDelay = 2
#if IS_CONTROLLER
+ 8
#endif //IS_CONTROLLER
;


#define READY_ROT 0x01
#define READY_BUT 0x02
#define READY_POS 0x04
#define READY_RAW 0x08
#define READY_CAL 0x10

uint8_t ReadyMask = 0;
uint8_t ProcessMask = 0;

uint32_t LastUsbTransmit = 0;

bool hasRotationData = false;
bool hasPositionData = false;
uint8_t buttonRetransmit = 0;

//KalmanSingle AccelFilter[3] = {
//KalmanSingle(0.125,32,1,0),
//KalmanSingle(0.125,32,1,0),
//KalmanSingle(0.125,32,1,0)
//};

uint8_t SensorCalibRequest = 0xFF;
uint32_t eepromSaveTime = 0;

uint32_t buttonRefreshTimeout = 1000;
CSensorFusion orientFuse(0.2f);
CSensorFusion gravFuse(1.0f);

#ifdef __cplusplus
extern "C" {
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI2_Init(void);
static void MX_I2C2_Init(void);

/* USER CODE BEGIN PFP */
	/* Private function prototypes -----------------------------------------------*/

#ifdef __cplusplus
}
#endif


static void MX_ADC1_Init(void);

RF_InitTypeDef RF_InitStruct = { 0 };
unsigned char NRF24L01_Init(void)
{
	RF_InitStruct.RF_Power_State = RF_Power_On;
	RF_InitStruct.RF_Config = RF_Config_IRQ_RX_Off | RF_Config_IRQ_TX_Off | RF_Confing_IRQ_Max_Rt_Off;
	RF_InitStruct.RF_CRC_Mode = RF_CRC8_On;
	RF_InitStruct.RF_Mode = RF_Mode_RX;
	RF_InitStruct.RF_Pipe_Auto_Ack = 0;
	RF_InitStruct.RF_Enable_Pipe = 1;
	RF_InitStruct.RF_Setup = RF_Setup_5_Byte_Adress;
	RF_InitStruct.RF_TX_Power = RF_TX_Power_High;
	RF_InitStruct.RF_Data_Rate = RF_Data_Rate_2Mbs;
	RF_InitStruct.RF_Channel = 250;

#if IS_HMD
	RF_InitStruct.RF_TX_Adress[0] = 'C';
	RF_InitStruct.RF_TX_Adress[1] = 'T';
	RF_InitStruct.RF_TX_Adress[2] = 'R';
	RF_InitStruct.RF_TX_Adress[3] = 'L';
	RF_InitStruct.RF_TX_Adress[4] = '0';
	RF_InitStruct.RF_RX_Adress_Pipe0[0] = 'B';
	RF_InitStruct.RF_RX_Adress_Pipe0[1] = 'A';
	RF_InitStruct.RF_RX_Adress_Pipe0[2] = 'S';
	RF_InitStruct.RF_RX_Adress_Pipe0[3] = 'E';
	RF_InitStruct.RF_RX_Adress_Pipe0[4] = '1'; //HMD
#endif //IS_HMD

#if IS_CONTROLLER
	RF_InitStruct.RF_TX_Adress[0] = 'B';
	RF_InitStruct.RF_TX_Adress[1] = 'A';
	RF_InitStruct.RF_TX_Adress[2] = 'S';
	RF_InitStruct.RF_TX_Adress[3] = 'E';
	RF_InitStruct.RF_TX_Adress[4] = '1';
	RF_InitStruct.RF_RX_Adress_Pipe0[0] = 'C';
	RF_InitStruct.RF_RX_Adress_Pipe0[1] = 'T';
	RF_InitStruct.RF_RX_Adress_Pipe0[2] = 'R';
	RF_InitStruct.RF_RX_Adress_Pipe0[3] = 'L';
	RF_InitStruct.RF_RX_Adress_Pipe0[4] = '0';
#endif //IS_CONTROLLER

	RF_InitStruct.RF_Payload_Size_Pipe0 = 32;
	RF_InitStruct.RF_Auto_Retransmit_Count = 0;
	RF_InitStruct.RF_Auto_Retransmit_Delay = 0;
	return RF_Init(&hspi2, &RF_InitStruct);
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
	while ((HAL_GetMicros() - tickstart) < delay) {}
}

inline uint8_t sgn(float n)
{
	return (n < 0 ? -1 : 1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case IR_SENS0_Pin:
		break;
	case IR_SENS1_Pin:
		break;
	case IR_SENS2_Pin:
		break;
		//case IR_SENS3_Pin: disabled for SWJ-IO
		//	break;
	case IR_SENS4_Pin:
		break;
	case IR_SENS5_Pin:
		break;
	case CTL_BTN0_Pin:
	case CTL_BTN1_Pin:
	case CTL_BTN2_Pin:
		//first 3 pins are for IPD rotary knob if device is HMD
	case CTL_BTN3_Pin:
	case CTL_BTN4_Pin:
	case CTL_BTN5_Pin:
	case CTL_BTN6_Pin:
	case CTL_BTN7_Pin:
		LastDigitalChange = HAL_GetTick();
		break;
	}
}


inline void ProcessOwnPacket(USBPacket *pPacket)
{
	if (lastCommandSequence != pPacket->Header.Sequence) //discard duplicates
	{
		lastCommandSequence = pPacket->Header.Sequence;
		switch (pPacket->Command.Command)
		{
		case CMD_RAW_DATA:
			feedRawMode = (RawModes)pPacket->Command.Data.Raw.State;
			break;
		case CMD_VIBRATE:
#if IS_CONTROLLER
			if (pPacket->Command.Data.Vibration.Axis == 0)
			{
				vibrationStopTime0 = ticks + pPacket->Command.Data.Vibration.Duration;
				HAL_GPIO_WritePin(CTL_VIBRATE0_GPIO_Port, CTL_VIBRATE0_Pin, GPIO_PIN_SET);
			}
			else if (pPacket->Command.Data.Vibration.Axis == 1)
			{
				vibrationStopTime1 = ticks + pPacket->Command.Data.Vibration.Duration;
				HAL_GPIO_WritePin(CTL_VIBRATE1_GPIO_Port, CTL_VIBRATE1_Pin, GPIO_PIN_SET);
			}
#endif //IS_CONTROLLER
			break;
		case CMD_CALIBRATE:
			//manually set sensor offsets
			int sensorId = pPacket->Command.Data.Calibration.Sensor;
			if (pPacket->Command.Data.Calibration.Command == CALIB_GET)
			{
				SensorCalibRequest = sensorId;
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{
					position.Position[i] = 0;
					VelocityVector[i] = 0;
					//AccelVector[i] = 0;
					CompAccelVector[i] = 0;
				}
				if (sensorId == SENSOR_NONE)
					break;
				else if (sensorId == SENSOR_ACCEL)
					eepromData.Accel = pPacket->Command.Data.Calibration;
				else if (sensorId == SENSOR_GYRO)
					eepromData.Gyro = pPacket->Command.Data.Calibration;
				else if (sensorId == SENSOR_MAG)
					eepromData.Mag = pPacket->Command.Data.Calibration;
				eepromSaveTime = ticks + 1000; //save after 1 second
			}
			break;
		}
	}
	pPacket->Header.Type = 0xff;
}

#if IS_CONTROLLER
inline void ForwardRFPacket(USBPacket *pPacket)
{
	RF_SendPayload(&hspi2, (uint8_t*)pPacket, sizeof(USBPacket));
}
#endif //IS_CONTROLLER

#if IS_HMD
inline void ForwardUSBPacket(USBPacket *pPacket)
{
	uint8_t channelIndex = pPacket->Header.Type & 0x0F;
	if (channelIndex < MAX_SOURCE)
	{
		CommChannel *pChannel = &Channels[channelIndex];
		pChannel->lastReceive = now;
		//if (channelIndex == HMD_SOURCE)
		//	pChannel->lastInSequence = pPacket->Header.Sequence - 1;
		if (pChannel->lastInSequence != pPacket->Header.Sequence) //skip duplicates for same channel
		{
			blinkDelay = 100;
			pChannel->lastInSequence = pPacket->Header.Sequence;
			USBD_CUSTOM_HID_SendReport_FS((uint8_t*)pPacket, (uint16_t) sizeof(USBPacket)); //forward incoming	
			LastUsbTransmit = HAL_GetTick();
		}
		else
			return;
	}
}

inline void BroadcastRFPackets()
{
	//send everything in channel buffer without checking sequence
	for (uint8_t channelIndex = 0; channelIndex < MAX_SOURCE; channelIndex++)
	{
		if (!(ChannelStateFlags & (1 << channelIndex)))
			continue; // skip if channel is empty			
		CommChannel *pChannel = &Channels[channelIndex];
		if (!pChannel->remainingTransmits)
		{
			// skip if no more to transmit. unlikely to enter here
			memset(&pChannel->lastCommand, 0, sizeof(USBPacket));
			ChannelStateFlags &= ~(1 << channelIndex); //clear flags
			continue;
		}
		if (now - pChannel->lastReceive > 10000)
		{
			// skip if timedout. (powered off controller etc)
			memset(&pChannel->lastCommand, 0, sizeof(USBPacket));
			ChannelStateFlags &= ~(1 << channelIndex); //clear flags
			continue;
		}

		if (!readyToSend)
		{
			rfStatus = RF_FifoStatus(&hspi2);
			readyToSend = ((rfStatus & RF_TX_FIFO_EMPTY_Bit) == RF_TX_FIFO_EMPTY_Bit); //if empty 
		}
		if (readyToSend && pChannel->remainingTransmits > 0 && now > pChannel->nextTransmit)
		{
			readyToSend = false;
			pChannel->nextTransmit = HAL_GetTick() + (rand() % 10); //max 10 ms for retransmit
			//switch to tx mode and send command to device			
			RF_TransmitMode(&hspi2, RF_InitStruct.RF_TX_Adress);
			RF_SendPayload(&hspi2, (uint8_t *)&pChannel->lastCommand, sizeof(USBPacket));
			do { rfStatus = RF_FifoStatus(&hspi2); } while ((rfStatus & RF_TX_FIFO_EMPTY_Bit) != RF_TX_FIFO_EMPTY_Bit); //wait for send complete
			pChannel->remainingTransmits--;
			if (!pChannel->remainingTransmits)
			{
				memset(&pChannel->lastCommand, 0, sizeof(USBPacket));
				ChannelStateFlags &= ~(1 << channelIndex); //clear flags
			}
			RF_ReceiveMode(&hspi2, RF_InitStruct.RF_RX_Adress_Pipe0);
			break; //send single packet only
		}
	}
}
#endif //IS_HMD

inline void ForwardPacket(USBPacket *pPacket)
{
#if IS_HMD
	ForwardUSBPacket(pPacket);
#endif //IS_HMD				
#if IS_CONTROLLER				
	ForwardRFPacket(pPacket);
#endif //IS_CONTROLLER				
}

inline void ProcessSensors()
{
	float elapsedTime = (now - lastRotationUpdate) / 1000000.0f; // set integration time by time elapsed since last filter update				

	if (elapsedTime >= 0.005f) //update at 200hz
	{
		hasRotationData |= readAccelData(tSensorData.Accel.Raw);  // Read the x/y/z adc values			
		hasRotationData |= readGyroData(tSensorData.Gyro.Raw);
		hasRotationData |= readMagData(tSensorData.Mag.Raw);


		if (hasRotationData) // recalc position if still has position data to be sent
		{
			ReadyMask |= READY_ROT;
			tSensorData.Accel.ProcessNew();
			tSensorData.Gyro.ProcessNew();
			tSensorData.Mag.ProcessNew();

			tSensorData.TimeElapsed = elapsedTime;
			lastRotationUpdate = now;
			orientQuat = orientFuse.FuseOrient(&tSensorData);

			//gravQuat = orientQuat;
//				gravQuat.w = 0; gravQuat.x = 0; gravQuat.y = 0; gravQuat.z = 0;
//				gravQuat = (orientQuat * gravQuat) * orientQuat.conjugate();

			if (isPositionReady && (hasPositionData || (isCalibrated && (ticks >= 10000))))
			{
				//float elapsedPositionTime = (float)(now - lastPositionIntegrateTime) / 1000000.0f;
				//if (elapsedPositionTime >= 0.001f) // integrate every 2 ms
				{
					GravityVector[0] = 2.0f * (orientQuat.x * orientQuat.z - orientQuat.w * orientQuat.y);
					GravityVector[1] = 2.0f * (orientQuat.w * orientQuat.x + orientQuat.y * orientQuat.z);
					GravityVector[2] = orientQuat.w * orientQuat.w - orientQuat.x * orientQuat.x - orientQuat.y * orientQuat.y + orientQuat.z * orientQuat.z;

					for (int i = 0; i < 3; i++)
					{
						float prevAccel = CompAccelVector[i];
						float prevVelocity = VelocityVector[i];
						float newAccel = tSensorData.Accel.Converted[i] - GravityVector[i];
						float newVelocity = ((prevAccel + newAccel) / 2.0f) * 9.81f * elapsedTime;
						float prevPosition = position.Position[i];

						//if velocity changes direction hold for a moment
//							if (sgn(prevVelocity) != sgn(newVelocity))
//							{
//								newVelocity = prevVelocity = 0;
//								//newAccel = 0;
//							}


						position.Position[i] = prevPosition + ((prevVelocity + newVelocity) / 2.0f) * elapsedTime * 10.0f;
						CompAccelVector[i] = newAccel;
						VelocityVector[i] = prevVelocity + newVelocity;
					}

					//if (now - lastPositionSend >= 20000) //50 ms
					{
						positionQuat.w = 0;
						positionQuat.x = position.Position[0];
						positionQuat.y = position.Position[1];
						positionQuat.z = position.Position[2];

						positionQuat = (orientQuat * positionQuat) * orientQuat.conjugate();

						hasPositionData = true;
						ReadyMask |= READY_POS;
						//lastPositionSend = now;	
					}

					//lastPositionIntegrateTime = now;
				}
			}
			//else
			//	lastPositionIntegrateTime = now;
		}
	}
}

#if IS_CONTROLLER
inline void ProcessADC()
{
	if (ticks - LastADCChange >= 50) //20 analog values per second
	{
		uint8_t changed = 0;
		LastADCChange = ticks;
		ADC_Value[0] = (((float)(ADC_Values[0] + ADC_Values[2]) / 2.0f) * 0.5f) + (ADC_Value[0] * 0.5f);
		ADC_Value[1] = (((float)(ADC_Values[1] + ADC_Values[3]) / 2.0f) * 0.5f) + (ADC_Value[1] * 0.5f);

		uint32_t value0 = ((uint32_t)(ADC_Value[0] / 40.96f));
		uint32_t value1 = ((uint32_t)(ADC_Value[1] / 40.96f));

		if (ADC_Cache[0] != value0) { changed = 1; ADC_Cache[0] = value0; }
		if (ADC_Cache[1] != value1) { changed = 1; ADC_Cache[1] = value1; }
		if (changed)
		{
			//if any analog value has changes
			buttonRetransmit = 5;
			lastButtonRefresh = ticks;
		}
	}
}
#endif //IS_CONTROLLER

inline void CalibrateSensors(void)
{
	for (int i = 0; i < 3; i++)
	{
		if (eepromData.Accel.Sensor == SENSOR_ACCEL)
		{
			tSensorData.Accel.Offset[i] = (float)(eepromData.Accel.RawMax[i] + eepromData.Accel.RawMin[i]) / 2.0f;
			//tSensorData.Accel.PosScale[i] = eepromData.Accel.RawMax[i] == 0? 1.0f : (1.0f / aRes) / (float)(eepromData.Accel.RawMax[i] - tSensorData.Accel.Offset[i]);
			//tSensorData.Accel.NegScale[i] = eepromData.Accel.RawMin[i] == 0? 1.0f : (1.0f / aRes) / (float)(eepromData.Accel.RawMin[i] - tSensorData.Accel.Offset[i]);
		}
		if (eepromData.Gyro.Sensor == SENSOR_GYRO)
		{
			tSensorData.Gyro.Offset[i] = (float)(eepromData.Gyro.RawMax[i]);
			tSensorData.Gyro.PosScale[i] = 1.0f;
			tSensorData.Gyro.NegScale[i] = 1.0f;
		}
		if (eepromData.Mag.Sensor == SENSOR_MAG)
		{
			tSensorData.Mag.Offset[i] = (float)(eepromData.Mag.RawMax[i] + eepromData.Mag.RawMin[i]) / 2.0f;
			//tSensorData.Mag.PosScale[i] = eepromData.Mag.RawMax[i] == 0? 1.0f : (1.0f / mRes) / (float)(eepromData.Mag.RawMax[i] - tSensorData.Mag.Offset[i]);
			//tSensorData.Mag.NegScale[i] = eepromData.Mag.RawMin[i] == 0? 1.0f : (1.0f / mRes) / (float)(eepromData.Mag.RawMin[i] - tSensorData.Mag.Offset[i]);
		}
	}
	isCalibrated = true;
}

inline void ResetSensorCalibration()
{
	memset(&eepromData, 0, sizeof(eepromData));
	tSensorData.ResetCalibration();
	isCalibrated = false;
}



/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_I2C2_Init();

  /* USER CODE BEGIN 2 */

#if IS_HMD		

  /* USB CUSTOM_HID device Configuration Descriptor */
	__ALIGN_BEGIN static uint8_t myDesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __ALIGN_END =
	{
		0x09, /* bLength: Configuration Descriptor size */
		USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
		USB_CUSTOM_HID_CONFIG_DESC_SIZ,
		/* wTotalLength: Bytes returned */
		0x00,
		0x01,         /*bNumInterfaces: 1 interface*/
		0x01,         /*bConfigurationValue: Configuration value*/
		0x00,         /*iConfiguration: Index of string descriptor describing
			the configuration*/
		0xC0,         /*bmAttributes: bus powered */
		0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

		/************** Descriptor of CUSTOM HID interface ****************/
		/* 09 */
		0x09,         /*bLength: Interface Descriptor size*/
		USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
		0x00,         /*bInterfaceNumber: Number of Interface*/
		0x00,         /*bAlternateSetting: Alternate setting*/
		0x02,         /*bNumEndpoints*/
		0x03,         /*bInterfaceClass: CUSTOM_HID*/
		0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
		0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
		0,            /*iInterface: Index of string descriptor*/
		/******************** Descriptor of CUSTOM_HID *************************/
		/* 18 */
		0x09,         /*bLength: CUSTOM_HID Descriptor size*/
		CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
		0x11,         /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
		0x01,
		0x00,         /*bCountryCode: Hardware target country*/
		0x01,         /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
		0x22,         /*bDescriptorType*/
		USBD_CUSTOM_HID_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
		0x00,
		/******************** Descriptor of Custom HID endpoints ********************/
		/* 27 */
		0x07,          /*bLength: Endpoint Descriptor size*/
		USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

		CUSTOM_HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
		0x03,          /*bmAttributes: Interrupt endpoint*/
		CUSTOM_HID_EPIN_SIZE, /*wMaxPacketSize: 2 Byte max */
		0x00,
		0x01,          /*bInterval: Polling Interval (20 ms)*/
		/* 34 */

		0x07,	         /* bLength: Endpoint Descriptor size */
		USB_DESC_TYPE_ENDPOINT,	/* bDescriptorType: */
		CUSTOM_HID_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
		0x03,	/* bmAttributes: Interrupt endpoint */
		CUSTOM_HID_EPOUT_SIZE,	/* wMaxPacketSize: 2 Bytes max  */
		0x00,
		0x01,	/* bInterval: Polling Interval (20 ms) */
		/* 41 */
	};

	uint16_t usbDescLen = 0;
	uint8_t *usbDesc = USBD_CUSTOM_HID.GetFSConfigDescriptor(&usbDescLen);
	memcpy(usbDesc, myDesc, usbDescLen);
	//*usbDesc = *myDesc;
	MX_USB_DEVICE_Init();
#endif //IS_HMD

#if IS_CONTROLLER
	MX_ADC1_Init();
#endif //IS_CONTROLLER

	DigitalValues = DigitalCache = 0;
	LedOff();


	srand(THIS_DEVICE + HAL_GetTick());

	//load eeprom calibration data	
	char *eepromBytes = (char *)&eepromData;
	int addr = 0;
	for (int i = 0; i < sizeof(eepromData); i += sizeof(uint32_t))
	{
		*((uint32_t *)(&eepromBytes[i])) = readEEPROMWord(addr);
		addr += sizeof(uint32_t);
	}

	if (eepromData.Magic == VALID_EEPROM_MAGIC)
		CalibrateSensors();
	else
		ResetSensorCalibration();



#if IS_CONTROLLER
	HAL_GPIO_WritePin(CTL_VIBRATE0_GPIO_Port, CTL_VIBRATE0_Pin, GPIO_PIN_SET);
	BlinkRease(30);
	HAL_GPIO_WritePin(CTL_VIBRATE0_GPIO_Port, CTL_VIBRATE0_Pin, GPIO_PIN_RESET);
	HAL_Delay(250);
#endif //IS_CONTROLLER

	rfStatus = NRF24L01_Init();
	if (rfStatus == 0x00 || rfStatus == 0xff)
		Error_Handler();
	LedOff();


	sensorStatus = initSensors();
	if (!sensorStatus)
		Error_Handler();
	LedOff();

#if IS_CONTROLLER
	buttonRefreshTimeout = 250;
	HAL_GPIO_WritePin(CTL_VIBRATE0_GPIO_Port, CTL_VIBRATE0_Pin, GPIO_PIN_SET);
	BlinkRease(20);
	HAL_GPIO_WritePin(CTL_VIBRATE0_GPIO_Port, CTL_VIBRATE0_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);

	//start background adc conversion
	if (HAL_ADC_Start(&hadc1) != HAL_OK)
		Error_Handler();
	if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Values, 4) != HAL_OK)
		Error_Handler();
	HAL_Delay(100);
#endif //IS_CONTROLLER		

	tSensorData.Setup(aRes, gRes, mRes);


	//do some warmup
	for (int i = 0; i < 100; i++)
	{
		readAccelData(tSensorData.Accel.Raw);  // Read the x/y/z adc values		
		tSensorData.Accel.ProcessNew();
		HAL_Delay(1);
		readGyroData(tSensorData.Gyro.Raw);
		tSensorData.Gyro.ProcessNew();
		HAL_Delay(1);
		readMagData(tSensorData.Mag.Raw);
		tSensorData.Mag.ProcessNew();
		HAL_Delay(10);
	}

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while (true)
	{
		Blink(blinkDelay);
		blinkDelay = 1000;

		now = HAL_GetMicros();
		ticks = HAL_GetTick(); //now / 1000;

		if (eepromSaveTime && (ticks >= eepromSaveTime))
		{
			CalibrateSensors();
			eepromData.Magic = VALID_EEPROM_MAGIC;
			enableEEPROMWriting();
			char *eepromBytes = (char *)&eepromData;
			int addr = 0;
			for (int i = 0; i < sizeof(eepromData); i += sizeof(uint32_t))
			{
				writeEEPROMWord(addr, *((uint32_t *)(&eepromBytes[i])));
				addr += sizeof(uint32_t);
			}
			disableEEPROMWriting();
			eepromSaveTime = 0;
		}

#if IS_CONTROLLER
		if (vibrationStopTime0 && (ticks > vibrationStopTime0))
		{
			HAL_GPIO_WritePin(CTL_VIBRATE0_GPIO_Port, CTL_VIBRATE0_Pin, GPIO_PIN_RESET);
			vibrationStopTime0 = 0;
		}

		if (vibrationStopTime1 && (ticks > vibrationStopTime1))
		{
			HAL_GPIO_WritePin(CTL_VIBRATE1_GPIO_Port, CTL_VIBRATE1_Pin, GPIO_PIN_RESET);
			vibrationStopTime1 = 0;
		}
		ProcessADC();
#endif //IS_CONTROLLER

		if (LastDigitalChange && (ticks - LastDigitalChange >= 25)) //using sw debounce 
		{
			LastDigitalChange = 0;
			DigitalValues = 0;
			if (HAL_GPIO_ReadPin(CTL_BTN0_GPIO_Port, CTL_BTN0_Pin) == GPIO_PIN_SET) DigitalValues |= BUTTON_0;
			if (HAL_GPIO_ReadPin(CTL_BTN1_GPIO_Port, CTL_BTN1_Pin) == GPIO_PIN_SET) DigitalValues |= BUTTON_1;
			if (HAL_GPIO_ReadPin(CTL_BTN2_GPIO_Port, CTL_BTN2_Pin) == GPIO_PIN_SET) DigitalValues |= BUTTON_2;
			if (HAL_GPIO_ReadPin(CTL_BTN3_GPIO_Port, CTL_BTN3_Pin) == GPIO_PIN_SET) DigitalValues |= BUTTON_3;
			if (HAL_GPIO_ReadPin(CTL_BTN4_GPIO_Port, CTL_BTN4_Pin) == GPIO_PIN_SET) DigitalValues |= BUTTON_4;
			if (HAL_GPIO_ReadPin(CTL_BTN5_GPIO_Port, CTL_BTN5_Pin) == GPIO_PIN_SET) DigitalValues |= BUTTON_5;
			if (HAL_GPIO_ReadPin(CTL_BTN6_GPIO_Port, CTL_BTN6_Pin) == GPIO_PIN_SET) DigitalValues |= BUTTON_6;
			if (HAL_GPIO_ReadPin(CTL_BTN7_GPIO_Port, CTL_BTN7_Pin) == GPIO_PIN_SET) DigitalValues |= BUTTON_7;

			if (DigitalCache != DigitalValues)
			{
				//if any digital value has changes
				DigitalCache = DigitalValues;
				//hasTriggerData = true;
				buttonRetransmit = 1
#if IS_CONTROLLER						
					+ 4
#endif //IS_CONTROLLER
					;
				lastButtonRefresh = ticks;
			}
		}
#if IS_CONTROLLER		
		if (ticks - lastButtonRefresh >= buttonRefreshTimeout)
		{
			//update button status every 250ms.
			buttonRetransmit++;
			lastButtonRefresh = ticks;
		}
#endif //IS_CONTROLLER
		ProcessSensors();

#if IS_HMD
		//process only COMMAND packets from USB		
		if (((pFromUSBPacket->Header.Type & 0xF0) == COMMAND_DATA) && (pFromUSBPacket->Command.Command != CMD_NONE))
		{
			if (CheckPacketCrc(pFromUSBPacket))
			{
				uint8_t channelIndex = pFromUSBPacket->Header.Type & 0x0F;
				if (channelIndex == THIS_DEVICE)
				{
					ProcessOwnPacket(pFromUSBPacket);
				}
				else if (channelIndex < MAX_SOURCE)
				{
					//forward from USB to RF
					CommChannel *pChannel = &Channels[channelIndex];
					//only send if active more than 1 sec
					if (pChannel->lastOutSequence != pFromUSBPacket->Header.Sequence && now - pChannel->lastReceive <= 1000)
					{
						//got packet from usb, forward to trackeddevice
						memcpy(&pChannel->lastCommand, pFromUSBPacket, sizeof(USBPacket));
						//send same packet a few times
						pChannel->remainingTransmits = 5;
						pChannel->nextTransmit = now;
						pChannel->lastOutSequence = pFromUSBPacket->Header.Sequence;
						ChannelStateFlags |= 1 << channelIndex;
					}
					memset(pFromUSBPacket, 0, sizeof(USBPacket)); //clear incoming
				}
			}
		}
#endif //IS_HMD

		rfStatus = RF_Status(&hspi2);
		readyToReceive = (rfStatus & RF_RX_DR_IRQ_CLEAR) == RF_RX_DR_IRQ_CLEAR ? 1 : 0;

		if (readyToReceive)
		{
			rfStatus = RF_FifoStatus(&hspi2);
			readyToReceive = (rfStatus & RF_RX_FIFO_EMPTY_Bit) == 0x00 ? 1 : 0;
			rcvCounter = 0;
			while ((rcvCounter < 3) && readyToReceive) //max 3 packets 
			//if (readyToReceive) //one rf packet at a time
			{
				rcvCounter++;
				//receive command
				//has fifo, receive
				readyToReceive = RF_ReceivePayload(&hspi2, (uint8_t*)&FromRFPacket, sizeof(USBPacket)) == 0 ? 1 : 0; //if not empty 
				if (CheckPacketCrc(&FromRFPacket))
				{
					if (((FromRFPacket.Header.Type & 0x0F)) == THIS_DEVICE)
					{
						ProcessOwnPacket(&FromRFPacket);
					}
#if IS_HMD
					else
					{
						//forward from RF to USB
						while (HAL_GetTick() - LastUsbTransmit <= extraDelay) {};
						ForwardPacket(&FromRFPacket);
					}
#endif //IS_HMD	
				}
			}
		}

#if IS_HMD		
		BroadcastRFPackets();
#endif //IS_HMD	


		if (!ProcessMask)
		{
			if (buttonRetransmit)
				ReadyMask |= READY_BUT;
			if (feedRawMode)
				ReadyMask |= READY_RAW;
			if (SensorCalibRequest != 0xFF)
				ReadyMask |= READY_CAL;
			if (ReadyMask)
			{
				ProcessMask = ReadyMask;
				ReadyMask = 0;
			}
		}
		ticks = HAL_GetTick();
		if (ProcessMask && (ticks >= nextSend))
		{
#if IS_CONTROLLER
			rfStatus = RF_FifoStatus(&hspi2);
			readyToSend = (rfStatus & RF_TX_FIFO_FULL_Bit) == 0x00 ? 1 : 0;
#endif //IS_CONTROLLER			
#if IS_HMD
			while (HAL_GetTick() - LastUsbTransmit <= extraDelay) {};
			readyToSend = 1;
#endif //IS_HMD
		}

		if (readyToSend)
		{
#if IS_CONTROLLER			
			RF_TransmitMode(&hspi2, RF_InitStruct.RF_TX_Adress); //switch to tx mode
#endif //IS_CONTROLLER			
			//HAL_MicroDelay(10);			
			blinkDelay = 100;

			sndCounter++;

			if (ProcessMask & READY_CAL)
			{
				FromDevicePacket.Header.Type = THIS_DEVICE | COMMAND_DATA;
				FromDevicePacket.Header.Sequence++;
				FromDevicePacket.Command.Command = CMD_CALIBRATE;
				if (SensorCalibRequest == SENSOR_ACCEL)
					FromDevicePacket.Command.Data.Calibration = eepromData.Accel;
				else if (SensorCalibRequest == SENSOR_GYRO)
					FromDevicePacket.Command.Data.Calibration = eepromData.Gyro;
				else if (SensorCalibRequest == SENSOR_MAG)
					FromDevicePacket.Command.Data.Calibration = eepromData.Mag;
				FromDevicePacket.Command.Data.Calibration.Command = CALIB_GET;
				FromDevicePacket.Command.Data.Calibration.Sensor = SensorCalibRequest;
				SensorCalibRequest = 0xFF;
				SetPacketCrc(&FromDevicePacket);
				ForwardPacket(&FromDevicePacket);
				ProcessMask &= ~READY_CAL;
			}
			else if (ProcessMask & READY_RAW)
			{
				FromDevicePacket.Header.Type = THIS_DEVICE | COMMAND_DATA;
				FromDevicePacket.Header.Sequence++;
				FromDevicePacket.Command.Command = CMD_RAW_DATA;
				FromDevicePacket.Command.Data.Raw.State = feedRawMode;
				for (int idx = 0; idx < 3; idx++)
				{
					switch (feedRawMode)
					{
					case RawMode_Off:
						break;
					case RawMode_Raw:
					{
						//raw sensor values
						FromDevicePacket.Command.Data.Raw.Accel[idx] = tSensorData.Accel.Raw[idx];
						FromDevicePacket.Command.Data.Raw.Gyro[idx] = tSensorData.Gyro.Raw[idx];
						FromDevicePacket.Command.Data.Raw.Mag[idx] = tSensorData.Mag.Raw[idx];
						break;
					}
					case RawMode_Filtered:
					{
						//filtered sensor data
						FromDevicePacket.Command.Data.Raw.Accel[idx] = tSensorData.Accel.Filtered[idx];
						FromDevicePacket.Command.Data.Raw.Gyro[idx] = tSensorData.Gyro.Filtered[idx];
						FromDevicePacket.Command.Data.Raw.Mag[idx] = tSensorData.Mag.Filtered[idx];
						break;
					}
					case RawMode_Compensated:
					{
						//compensated sensor data
						FromDevicePacket.Command.Data.Raw.Accel[idx] = tSensorData.Accel.Compensated[idx];
						FromDevicePacket.Command.Data.Raw.Gyro[idx] = tSensorData.Gyro.Compensated[idx];
						FromDevicePacket.Command.Data.Raw.Mag[idx] = tSensorData.Mag.Compensated[idx];
						break;
					}
					case RawMode_DRVectors:
					{
						//deadreckoning vectors
						FromDevicePacket.Command.Data.Raw.Accel[idx] = (int16_t)(GravityVector[idx] / aRes);
						FromDevicePacket.Command.Data.Raw.Gyro[idx] = (int16_t)(CompAccelVector[idx] / aRes); //in m/s2
						FromDevicePacket.Command.Data.Raw.Mag[idx] = VelocityVector[idx] * 1000; //in mm/s
						break;
					}
					}
				}
				SetPacketCrc(&FromDevicePacket);
				ForwardPacket(&FromDevicePacket);
				ProcessMask &= ~READY_RAW;
			}
			else if (ProcessMask & READY_POS)
			{
				hasPositionData = false;
				FromDevicePacket.Header.Type = THIS_DEVICE | POSITION_DATA;
				FromDevicePacket.Header.Sequence++;

				//remap
				FromDevicePacket.Position.Position[0] = positionQuat.x; //position.Position[0];
				FromDevicePacket.Position.Position[1] = positionQuat.z; //position.Position[2];
				FromDevicePacket.Position.Position[2] = -positionQuat.y; //-position.Position[1];
				SetPacketCrc(&FromDevicePacket);
				ForwardPacket(&FromDevicePacket);
				ProcessMask &= ~READY_POS;
			}
			else if (ProcessMask & READY_ROT)
			{
				//send rot data
				hasRotationData = false;
				FromDevicePacket.Header.Type = THIS_DEVICE | ROTATION_DATA;
				FromDevicePacket.Header.Sequence++;
				//remap				
				FromDevicePacket.Rotation.w = orientQuat.w;
				FromDevicePacket.Rotation.x = orientQuat.x;
				FromDevicePacket.Rotation.y = orientQuat.z;
				FromDevicePacket.Rotation.z = -orientQuat.y;
				SetPacketCrc(&FromDevicePacket);
				ForwardPacket(&FromDevicePacket);
				ProcessMask &= ~READY_ROT;
			}
			else if (ProcessMask & READY_BUT)
			{
				buttonRetransmit--;
				FromDevicePacket.Header.Type = THIS_DEVICE | TRIGGER_DATA;
				FromDevicePacket.Header.Sequence++;
#if IS_HMD					
				FromDevicePacket.Trigger.Analog[0].x = 0;
				FromDevicePacket.Trigger.Analog[0].y = 0;
				FromDevicePacket.Trigger.Analog[1].x = 0; //reserved
				FromDevicePacket.Trigger.Analog[1].y = 0; //reserved
#endif //IS_HMD					

#if IS_CONTROLLER					
				FromDevicePacket.Trigger.Analog[0].x = ((float)ADC_Cache[0]) / 100.0f; //trigger
				FromDevicePacket.Trigger.Analog[0].y = ((float)ADC_Cache[1]) / 100.0f; //IPD
				FromDevicePacket.Trigger.Analog[1].x = 0; //reserved
				FromDevicePacket.Trigger.Analog[1].y = 0; //reserved
#endif //IS_CONTROLLER

				FromDevicePacket.Trigger.Digital = DigitalCache;
				SetPacketCrc(&FromDevicePacket);
				ForwardPacket(&FromDevicePacket);
				ProcessMask &= ~READY_BUT;
				//#if IS_CONTROLLER				
								//rfStatus = RF_FifoStatus(&hspi2); readyToSend = (rfStatus & RF_TX_FIFO_FULL_Bit) == 0x00 ? 1 : 0;									
				//#endif //IS_CONTROLLER				
			}

			if (!ProcessMask)
#if IS_CONTROLLER			
				nextSend = ticks + (extraDelay + (rand() % 10)); //for next RF packet
			do { rfStatus = RF_FifoStatus(&hspi2); } while ((rfStatus & RF_TX_FIFO_EMPTY_Bit) != RF_TX_FIFO_EMPTY_Bit); //wait for send complete			
#endif //IS_CONTROLLER			
#if IS_HMD
			nextSend = ticks + extraDelay; //for next USB packet
#endif //IS_HMD

			readyToSend = 0;
#if IS_CONTROLLER
			HAL_MicroDelay(10);
			RF_ReceiveMode(&hspi2, RF_InitStruct.RF_RX_Adress_Pipe0); //back to rx mode
#endif //IS_CONTROLLER			
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

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
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

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* I2C2 init function */
static void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

}

/* SPI2 init function */
static void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through 
        * the Code Generation settings)
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
  HAL_GPIO_WritePin(GPIOB, SPI_RF_NSS_Pin|SPI_RF_CE_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CTL_VIBRATE1_Pin|CTL_VIBRATE0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IR_SENS4_Pin IR_SENS5_Pin */
  GPIO_InitStruct.Pin = IR_SENS4_Pin|IR_SENS5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA2 PA13 PA14 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CTL_BTN0_Pin CTL_BTN1_Pin CTL_BTN2_Pin CTL_BTN3_Pin 
                           CTL_BTN4_Pin */
  GPIO_InitStruct.Pin = CTL_BTN0_Pin|CTL_BTN1_Pin|CTL_BTN2_Pin|CTL_BTN3_Pin 
                          |CTL_BTN4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CTL_BTN5_Pin CTL_BTN6_Pin CTL_BTN7_Pin */
  GPIO_InitStruct.Pin = CTL_BTN5_Pin|CTL_BTN6_Pin|CTL_BTN7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_SYNC_Pin */
  GPIO_InitStruct.Pin = IR_SYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IR_SYNC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IR_SENS0_Pin IR_SENS1_Pin IR_SENS2_Pin */
  GPIO_InitStruct.Pin = IR_SENS0_Pin|IR_SENS1_Pin|IR_SENS2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI_RF_NSS_Pin SPI_RF_CE_Pin */
  GPIO_InitStruct.Pin = SPI_RF_NSS_Pin|SPI_RF_CE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : CTL_VIBRATE1_Pin CTL_VIBRATE0_Pin */
  GPIO_InitStruct.Pin = CTL_VIBRATE1_Pin|CTL_VIBRATE0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
#ifdef __cplusplus
extern "C"
#endif
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
	BlinkRease2(80, false);
	LedOff();
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
