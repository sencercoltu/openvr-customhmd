#include "stm32f1xx_hal.h"

extern bool i2c_writeSingleByte(uint8_t addr, uint8_t data);
extern bool i2c_writeRegisterByte(uint8_t addr, uint8_t reg, uint8_t data);
extern bool i2c_readRegister(uint8_t addr, uint8_t reg, uint8_t* pResult);
extern bool i2c_readData(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size);
extern bool i2c_isDeviceReady(uint8_t addr);
extern bool i2c_isReady();
extern void i2c_wait();
