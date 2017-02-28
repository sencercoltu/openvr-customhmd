#ifndef _LED_H_
#define _LED_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include "stm32f1xx_hal.h"


extern void HAL_MicroDelay(uint64_t delay);

extern uint32_t led_ticks;

extern void LedOn(void);
extern void LedOff(void);
extern void LedToggle(void);
extern uint8_t Blink(int delay);
extern void BlinkDelay(int count, int delay);
extern void BlinkRease(int count);
extern void BlinkRease2(int count, bool reverse);
extern void LedIntensity(float i);

#ifdef __cplusplus
}
#endif

#endif //_LED_H_
