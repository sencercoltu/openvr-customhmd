#include "i2c.h"

#define NUNCHUCK_ADDRESS      0x52

extern uint8_t nunchuckId[6];
extern void initNunchuck();
extern void readNunchuck(uint8_t *data);
extern uint8_t nunchuk_decode_byte(uint8_t x);
