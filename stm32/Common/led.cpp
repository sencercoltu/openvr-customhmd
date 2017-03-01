#include "led.h"

uint32_t _ledTicks = 0;
GPIO_PinState _ledState = GPIO_PIN_SET;
float _ledIntensity = 100;

void HAL_SYSTICK_Callback(void)
{
	if (_ledIntensity == 0)
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); //turn off
	else if (_ledIntensity == 100)
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, _ledState); //turn off
	else
	{
		uint32_t isOff = HAL_GetTick() % (uint32_t)(100.0f / _ledIntensity);
		if (isOff)
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); //turn off
		else
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, _ledState); //turn off
		
	}
}

void LedIntensity(float i)
{
	if (i<0) _ledIntensity = 0;
	else if (i>100) _ledIntensity = 100;
	else _ledIntensity = i;
}

void LedOn(void)
{
	_ledState = GPIO_PIN_RESET;
	//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); //ters calisiyo led
}

void LedOff(void)
{
	_ledState = GPIO_PIN_SET;
	//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); //ters calisiyo led
}

void BlinkRease(int count)
{
	BlinkRease2(count, false);
}

void BlinkRease2(int count, bool reverse)
{		
	int cnt = count;
	if (reverse)
		while (cnt > 0)
		{
			LedToggle();
			HAL_Delay(count - (cnt--));				
		}
	else
		while (cnt > 0)
		{
			LedToggle();
			HAL_Delay(cnt--);				
		}
}

void LedToggle(void)
{
	if (_ledState == GPIO_PIN_RESET)
		_ledState = GPIO_PIN_SET;
	else
		_ledState = GPIO_PIN_RESET;
	//HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

uint8_t Blink(int delay = 100)
{
	uint32_t ticks = HAL_GetTick();
	if (ticks - _ledTicks >= delay)
	{
		LedToggle();
		_ledTicks = ticks;
	}
	return 1;
}

void BlinkDelay(int count, int delay)
{
	for (int i = 0; i < count; i++)
	{
		LedToggle();
		HAL_Delay(delay);
	}
}

