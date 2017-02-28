#ifndef _L3G4200_H_
#define _L3G4200_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"
#include "led.h"

#define WHO_AM_I_L3G4200D       0x0F  // Should return 0xD3
#define L3G4200D_CTRL_REG1      0x20
#define L3G4200D_CTRL_REG2      0x21
#define L3G4200D_CTRL_REG3      0x22
#define L3G4200D_CTRL_REG4      0x23
#define L3G4200D_CTRL_REG5      0x24
#define L3G4200D_REFERENCE      0x25
#define L3G4200D_OUT_TEMP       0x26
#define L3G4200D_STATUS_REG     0x27
#define L3G4200D_OUT_X_L        0x28
#define L3G4200D_OUT_X_H        0x29
#define L3G4200D_OUT_Y_L        0x2A
#define L3G4200D_OUT_Y_H        0x2B
#define L3G4200D_OUT_Z_L        0x2C
#define L3G4200D_OUT_Z_H        0x2D
#define L3G4200D_FIFO_CTRL_REG  0x2E
#define L3G4200D_FIFO_SRC_REG   0x2F
#define L3G4200D_INT1_CFG       0x30
#define L3G4200D_INT1_SRC       0x31
#define L3G4200D_INT1_TSH_XH    0x32
#define L3G4200D_INT1_TSH_XL    0x33
#define L3G4200D_INT1_TSH_YH    0x34
#define L3G4200D_INT1_TSH_YL    0x35
#define L3G4200D_INT1_TSH_ZH    0x36
#define L3G4200D_INT1_TSH_ZL    0x37
#define L3G4200D_INT1_DURATION  0x38
#define L3G4200D_ADDRESS        0x69  // Device address when ADO = 0


enum L3G4200D_Scale {
  GFS_250DPS = 0,
  GFS_500DPS,
  GFS_1000DPS,
  GFS_2000DPS
};

enum L3G4200D_Rate { // set gyro ODR and Bandwidth with 4 bits
  GRTBW_100_125 = 0, // 100 Hz ODR, 12.5 Hz bandwidth
  GRTBW_100_25,
  GRTBW_100_25a,
  GRTBW_100_25b,
  GRTBW_200_125,
  GRTBW_200_25,
  GRTBW_200_50,
  GRTBW_200_70,
  GRTBW_400_20,
  GRTBW_400_25,
  GRTBW_400_50,
  GRTBW_400_110,
  GRTBW_800_30,
  GRTBW_800_35,
  GRTBW_800_50,
  GRTBW_800_110  // 800 Hz ODR, 110 Hz bandwidth   
};

void initL3G4200D();
bool checkL3G4200D();
bool readL3G4200DData(int16_t *destination);
float getL3G4200Dres();

#ifdef __cplusplus
}
#endif

#endif //_L3G4200_H_
