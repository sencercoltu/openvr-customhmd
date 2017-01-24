#include "eeprom_flash.h"

/*
 * Must call this first to enable writing
 */
void enableEEPROMWriting() {
    HAL_StatusTypeDef status = HAL_FLASH_Unlock();
    FLASH_PageErase(EEPROM_START_ADDRESS); // required to re-write
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER); // Bug fix: bit PER has been set in Flash_PageErase(), must clear it here
}

void disableEEPROMWriting() {
    HAL_FLASH_Lock();
}

/*
 * Writing functions
 * Must call enableEEPROMWriting() first
 */
HAL_StatusTypeDef writeEEPROMHalfWord(uint32_t address, uint16_t data) {
    HAL_StatusTypeDef status;
    address = address + EEPROM_START_ADDRESS;
    
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data);
    
    return status;
}

HAL_StatusTypeDef writeEEPROMWord(uint32_t address, uint32_t data) {
    HAL_StatusTypeDef status;
    address = address + EEPROM_START_ADDRESS;
    
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);

    return status;
}

/*
 * Reading functions
 */
uint16_t readEEPROMHalfWord(uint32_t address) {
    uint16_t val = 0;
    address = address + EEPROM_START_ADDRESS;
    val = *(__IO uint16_t*)address;
    
    return val;
}

uint32_t readEEPROMWord(uint32_t address) {
    uint32_t val = 0;
    address = address + EEPROM_START_ADDRESS;
    val = *(__IO uint32_t*)address;
    
    return val;
}
