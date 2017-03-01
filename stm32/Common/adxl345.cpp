#include "adxl345.h"

ADXL345_Scale ADXL345Ascale = AFS_2G;
ADXL345_Rate ADXL345Arate = ARTBW_200_100; // 200 Hz ODR, 100 Hz bandwidth

void initADXL345()
{
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_POWER_CTL, 0x00); // Put device in standby mode and clear sleep bit 2
	HAL_Delay(100);  // Let device settle down
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_POWER_CTL, 0x08); // Put device in normal mode
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_DATA_FORMAT, 0x04 | ADXL345Ascale); // Set full scale range for the accelerometer 
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_BW_RATE, ADXL345Arate); // Select normal power operation, and ODR and bandwidth
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_INT_MAP, 0x00);     // Enable data ready interrupt on INT_2
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_INT_ENABLE, 0x00);  // Enable data ready interrupt
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 0x00);    // Bypass FIFO	
}

void calibrateADXL345()
{
	uint8_t data[6] = { 0, 0, 0, 0, 0, 0 };
	int abias[3] = { 0, 0, 0 };
	int16_t accel_bias[3] = { 0, 0, 0 };
	uint8_t samples, ii;

	// wake up device
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_POWER_CTL, 0x00); // Put device in standby mode and clear sleep bit 2
	HAL_Delay(10);  // Let device settle down
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_POWER_CTL, 0x08); // Put device in normal mode
	HAL_Delay(10);

	// Set accelerometer configuration; interrupt active high, left justify MSB
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_DATA_FORMAT, 0x04 | 0x00); // Set full scale range to 2g for the bias calculation 
	uint16_t  accelsensitivity = 256;  // = 256 LSB/g at 2g full scale

									   // Choose ODR and bandwidth
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_BW_RATE, 0x09); // Select normal power operation, and 100 Hz ODR and 50 Hz bandwidth
	HAL_Delay(10);

	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 0x40 | 0x2F);    // Enable FIFO stream mode | collect 32 FIFO samples
	HAL_Delay(1000);  // delay 1000 milliseconds to collect FIFO samples

	i2c_readRegister(ADXL345_ADDRESS, ADXL345_FIFO_STATUS, &samples);
	for (ii = 0; ii < samples; ii++) {
		i2c_readData(ADXL345_ADDRESS, ADXL345_DATAX0, data, 6);
		accel_bias[0] += (((int16_t)data[1] << 8) | data[0]) >> 6;
		accel_bias[1] += (((int16_t)data[3] << 8) | data[2]) >> 6;
		accel_bias[2] += (((int16_t)data[5] << 8) | data[4]) >> 6;
	}

	accel_bias[0] /= samples; // average the data
	accel_bias[1] /= samples;
	accel_bias[2] /= samples;

	// Remove gravity from z-axis accelerometer bias value
	if (accel_bias[2] > 0) {
		accel_bias[2] -= accelsensitivity;
	}
	else {
		accel_bias[2] += accelsensitivity;
	}

	abias[0] = (int)((-accel_bias[0] / 4) & 0xFF); // offset register are 8 bit 2s-complement, so have sensitivity 1/4 of 2g full scale
	abias[1] = (int)((-accel_bias[1] / 4) & 0xFF);
	abias[2] = (int)((-accel_bias[2] / 4) & 0xFF);

	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_OFSX, abias[0]);
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_OFSY, abias[1]);
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_OFSZ, abias[2]);
}

void resetADXL345Offset()
{
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_POWER_CTL, 0x00); // Put device in standby mode and clear sleep bit 2
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_OFSX, 0);
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_OFSY, 0);
	i2c_writeRegisterByte(ADXL345_ADDRESS, ADXL345_OFSZ, 0);
}

bool checkADXL345()
{
	LedOff();	
	if (i2c_isDeviceReady(ADXL345_ADDRESS))
	{	
		uint8_t c;
		i2c_readRegister(ADXL345_ADDRESS, WHO_AM_I_ADXL345, &c);  // Read WHO_AM_I register for ADXL345	
		if (c == 0xE5)
			return true;
		BlinkDelay(10, 500);
	}	
	return false;
}

bool readADXL345Data(int16_t *destination)
{
	uint8_t res = 0;
	if (i2c_readRegister(ADXL345_ADDRESS, ADXL345_INT_SOURCE, &res) && (res & 0x80))
	{
		uint8_t rawData[6];  // x/y/z accel register data stored here
		i2c_readData(ADXL345_ADDRESS, ADXL345_DATAX0, rawData, 6);  // Read the six raw data registers into data array
		destination[0] = (((int16_t)rawData[1]) << 8) | rawData[0];  // Turn the MSB and LSB into a signed 16-bit value
		destination[1] = (((int16_t)rawData[3]) << 8) | rawData[2];
		destination[2] = (((int16_t)rawData[5]) << 8) | rawData[4];
		return true;
	}
	return false;
}

float getADXL345res()
{
	switch (ADXL345Ascale)
	{
		// Possible accelerometer scales (and their register bit settings) are:
		// 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11). 
	case AFS_2G:
		return 2.0f / 32768.0f;   // 10-bit 2s-complement		
	case AFS_4G:
		return 4.0f / 32768.0f;  // 11-bit 2s-complement		
	case AFS_8G:
		return 8.0f / 32768.0f;  // 12-bit 2s-complement		
	case AFS_16G:
		return 16.0f / 32768.0f;  // 13-bit 2s-complement		
	}
	return 0.0f;
}

