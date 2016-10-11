#include "stm32f1xx_hal.h"

extern void HAL_MicroDelay(uint64_t delay);

extern uint32_t led_ticks;

extern void LedOn();
extern void LedOff();
extern void LedToggle();
extern uint8_t Blink(int delay);
extern void BlinkDelay(int count, int delay);
extern void BlinkRease(int count);
extern void BlinkRease(int count, bool reverse);
extern void LedIntensity(float i);
