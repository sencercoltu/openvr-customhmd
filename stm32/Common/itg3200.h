#ifndef _IT3200_H_
#define _IT3200_H_

#ifdef __cplusplus
extern "C" {
#endif

	
#include "i2c.h"
#include "led.h"


#define ITG3200_ADDRESS_AD0_LOW     0x68 // address pin low (GND), default for SparkFun IMU Digital Combo board
#define ITG3200_ADDRESS_AD0_HIGH    0x69 // address pin high (VCC), default for SparkFun ITG-3200 Breakout board
#define ITG3200_ADDRESS     ITG3200_ADDRESS_AD0_LOW

#define WHO_AM_I_ITG3200           0x00
#define ITG3200_RA_SMPLRT_DIV       0x15
#define ITG3200_RA_DLPF_FS          0x16
#define ITG3200_RA_INT_CFG          0x17
#define ITG3200_RA_INT_STATUS       0x1A
#define ITG3200_RA_TEMP_OUT_H       0x1B
#define ITG3200_RA_TEMP_OUT_L       0x1C
#define ITG3200_RA_GYRO_XOUT_H      0x1D
#define ITG3200_RA_GYRO_XOUT_L      0x1E
#define ITG3200_RA_GYRO_YOUT_H      0x1F
#define ITG3200_RA_GYRO_YOUT_L      0x20
#define ITG3200_RA_GYRO_ZOUT_H      0x21
#define ITG3200_RA_GYRO_ZOUT_L      0x22
#define ITG3200_RA_PWR_MGM          0x3E


enum ITG3200_Rate { // set gyro ODR and Bandwidth with 4 bits
	DLPF_BW_256,
	DLPF_BW_188,
	DLPF_BW_98,
	DLPF_BW_42,
	DLPF_BW_20,
	DLPF_BW_10,
	DLPF_BW_5
};

void initITG3200();
bool checkITG3200();
bool readITG3200Data(int16_t *destination);
float getITG3200res();

#ifdef __cplusplus
}
#endif

#endif //_IT3200_H_
