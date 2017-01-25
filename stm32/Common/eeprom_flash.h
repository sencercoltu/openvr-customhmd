#ifndef __EEPROM_FLASH_H
#define __EEPROM_FLASH_H

#include "stm32f1xx_hal.h"

//#define EEPROM_START_ADDRESS   ((uint32_t)0x08019000) // EEPROM emulation start address: after 100KByte of used Flash memory
//#define EEPROM_START_ADDRESS   ((uint32_t)0x0801C000) // EEPROM emulation start address: after 112KByte of used Flash memory
#define EEPROM_START_ADDRESS   ((uint32_t)0x0800F000) // EEPROM emulation start address: after 112KByte of used Flash memory

void enableEEPROMWriting(void); // Unlock and keep PER cleared
void disableEEPROMWriting(void); // Lock

// Write functions
HAL_StatusTypeDef writeEEPROMHalfWord(uint32_t address, uint16_t data);
HAL_StatusTypeDef writeEEPROMWord(uint32_t address, uint32_t data);

// Read functions
uint16_t readEEPROMHalfWord(uint32_t address);
uint32_t readEEPROMWord(uint32_t address);

#endif
