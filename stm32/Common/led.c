#include "led.h"

uint32_t led_ticks = 0;

	
void LedOn()
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); //ters çalisiyo led
}

void LedOff()
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); //ters çalisiyo led
}

void BlinkRease(int count)
{
	BlinkRease(count, false);
}

void BlinkRease(int count, bool reverse)
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

void LedToggle()
{
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

uint8_t Blink(int delay = 100)
{
	uint32_t ticks = HAL_GetTick();
	if (ticks - led_ticks >= delay)
	{
		LedToggle();
		led_ticks = ticks;
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
