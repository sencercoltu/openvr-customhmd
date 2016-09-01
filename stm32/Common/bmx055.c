#include "bmx055.h"

uint8_t Gscale = GFS55_500DPS;       // set gyro full scale  
uint8_t GODRBW = G55_400Hz47Hz;      // set gyro ODR and bandwidth 
uint8_t Ascale = AFS_2G;           // set accel full scale  
uint8_t ACCBW  = 0x08 | ABW_500Hz;  // Choose bandwidth for accelerometer, need bit 3 = 1 to enable bandwidth choice in enum
uint8_t Mmode  = lowPower;          // Choose magnetometer operation mode
uint8_t MODR   = MODR_10Hz;        // set magnetometer data rate 

int16_t magMaxX = -32000, magMinX = 32000;
int16_t magMaxY = -32000, magMinY = 32000;
int16_t magMaxZ = -32000, magMinZ = 32000;

bool checkBMX055()
{
	//HAL_Delay(100); //extra delay for bmx055
	LedOff();
	
	if (i2c_isDeviceReady(BMX055_ACC_ADDRESS) && i2c_isDeviceReady(BMX055_GYRO_ADDRESS) && i2c_isDeviceReady(BMX055_MAG_ADDRESS))
	{	
		uint8_t c;
		//BlinkDelay(15, 30);	
		c = 0;
	//	i2c_readRegister(BMX055_ACC_ADDRESS, BMX055_ACC_WHOAMI, &c);  // Read WHO_AM_I register for ADXL345	
	//	HAL_Delay(100); //extra delay for bmx055
	//	BlinkDelay(15, 30);
		i2c_readRegister(BMX055_ACC_ADDRESS, BMX055_ACC_WHOAMI, &c);  // Read WHO_AM_I register for ADXL345	
		if (c != 0xFA)
		{
			BlinkDelay(10, 500);
			return false;
		}
		c = 0;
		i2c_readRegister(BMX055_GYRO_ADDRESS, BMX055_GYRO_WHOAMI, &c);  
		if (c != 0x0F)
		{
			BlinkDelay(10, 500);
			return false;
		}	
		c = 0;
		i2c_writeRegisterByte(BMX055_MAG_ADDRESS, BMX055_MAG_PWR_CNTL1, 0x01);  
		HAL_Delay(100); 
		i2c_readRegister(BMX055_MAG_ADDRESS, BMX055_MAG_WHOAMI, &c);  
		if (c != 0x32)
		{
			BlinkDelay(10, 500);
			return false;
		}		
		return true;
	}
	return false;
}

// Parameters to hold BMX055 trim values
signed char   dig_x1;
signed char   dig_y1;
signed char   dig_x2;
signed char   dig_y2;
uint16_t      dig_z1;
int16_t       dig_z2;
int16_t       dig_z3;
int16_t       dig_z4;
unsigned char dig_xy1;
signed char   dig_xy2;
uint16_t      dig_xyz1;


//void trimBMX055()  // get trim values for magnetometer sensitivity
//{ 
//	uint8_t res = 0;
//	uint8_t rawData[2];  //placeholder for 2-byte trim data
//	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_X1, &res);  dig_x1 = res;
//	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_X2, &res); dig_x2 = res;
//	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_Y1, &res); dig_y1 = res;
//	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_Y2, &res); dig_y2 = res;
//	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_XY1, &res); dig_xy1 = res;
//	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_XY2, &res); dig_xy2 = res;
//    i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_Z1_LSB, rawData, 2);   
//	dig_z1 = (uint16_t) (((uint16_t)rawData[1] << 8) | rawData[0]);  
//    i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_Z2_LSB, rawData, 2);   
//	dig_z2 = (int16_t) (((int16_t)rawData[1] << 8) | rawData[0]);  
//    i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_Z3_LSB, rawData ,2);   
//	dig_z3 = (int16_t) (((int16_t)rawData[1] << 8) | rawData[0]);  
//    i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_Z4_LSB, rawData, 2);   
//	dig_z4 = (int16_t) (((int16_t)rawData[1] << 8) | rawData[0]);  
//    i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_XYZ1_LSB, rawData, 2);   
//	dig_xyz1 = (uint16_t) (((uint16_t)rawData[1] << 8) | rawData[0]);  
//}

void trimBMX055()  // get trim values for magnetometer sensitivity
{ 
	uint8_t rawData[2];  //placeholder for 2-byte trim data
	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_X1, (uint8_t *)&dig_x1);
	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_X2, (uint8_t *)&dig_x2);
	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_Y1, (uint8_t *)&dig_y1);
	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_Y2, (uint8_t *)&dig_y2);
	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_XY1, (uint8_t *)&dig_xy1);
	i2c_readRegister(BMX055_ACC_ADDRESS, BMM050_DIG_XY2, (uint8_t *)&dig_xy2);
	
	i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_Z1_LSB, rawData, 2);   
	dig_z1 = (uint16_t) (((uint16_t)rawData[1] << 8) | rawData[0]);  
	i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_Z2_LSB, rawData, 2);   
	dig_z2 = (int16_t) (((int16_t)rawData[1] << 8) | rawData[0]);  
	i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_Z3_LSB, rawData, 2);   
	dig_z3 = (int16_t) (((int16_t)rawData[1] << 8) | rawData[0]);  
	i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_Z4_LSB, rawData, 2);   
	dig_z4 = (int16_t) (((int16_t)rawData[1] << 8) | rawData[0]);  
	i2c_readData(BMX055_MAG_ADDRESS, BMM050_DIG_XYZ1_LSB, rawData, 2);   
	dig_xyz1 = (uint16_t) (((uint16_t)rawData[1] << 8) | rawData[0]);  
}

void initBMX055()
{
   i2c_writeRegisterByte(BMX055_ACC_ADDRESS,  BMX055_ACC_BGW_SOFTRESET, 0xB6);  // reset accelerometer
   HAL_Delay(1000); // Wait for all registers to reset 

   // Configure accelerometer
   i2c_writeRegisterByte(BMX055_ACC_ADDRESS, BMX055_ACC_PMU_RANGE, Ascale & 0x0F); // Set accelerometer full range
   i2c_writeRegisterByte(BMX055_ACC_ADDRESS, BMX055_ACC_PMU_BW, ACCBW & 0x0F);     // Set accelerometer bandwidth
   i2c_writeRegisterByte(BMX055_ACC_ADDRESS, BMX055_ACC_D_HBW, 0x00);              // Use filtered data	
	
	HAL_Delay(100); 
	
	i2c_writeRegisterByte(BMX055_ACC_ADDRESS, BMX055_ACC_OFC_CTRL, 0x80); // set all accel offset compensation registers to zero
	i2c_writeRegisterByte(BMX055_ACC_ADDRESS, BMX055_ACC_OFC_SETTING, 0x20);  // set offset targets to 0, 0, and +1 g for x, y, z axes
	
	HAL_Delay(100); 
	
    i2c_writeRegisterByte(BMX055_GYRO_ADDRESS, BMX055_GYRO_RANGE, Gscale);  // set GYRO FS range
    i2c_writeRegisterByte(BMX055_GYRO_ADDRESS, BMX055_GYRO_BW, GODRBW);     // set GYRO ODR and Bandwidth
	
	HAL_Delay(100); 
	
	i2c_writeRegisterByte(BMX055_MAG_ADDRESS, BMX055_MAG_PWR_CNTL1, 0x82);  // Softreset magnetometer, ends up in sleep mode	
	HAL_Delay(100);
	
	i2c_writeRegisterByte(BMX055_MAG_ADDRESS, BMX055_MAG_PWR_CNTL1, 0x01); // Wake up magnetometer
	HAL_Delay(100);

	i2c_writeRegisterByte(BMX055_MAG_ADDRESS, BMX055_MAG_PWR_CNTL2, MODR << 3); // Normal mode	
		
	HAL_Delay(100); // Wait for all registers to reset 
		
	i2c_writeRegisterByte(BMX055_MAG_ADDRESS, BMX055_MAG_REP_XY, 1 /*23*/);  //  3 samples // 2x+1 = 47 sample avg for XY-axis
	i2c_writeRegisterByte(BMX055_MAG_ADDRESS, BMX055_MAG_REP_Z, 2 /*41*/);  // 3 samples //2x+1 = 83 sample avg for Z-axis

	HAL_Delay(100); // Wait for all registers to reset 
	
	trimBMX055();
}

float getBMX055resGyro() {
  switch (Gscale)
  {
 	// Possible gyro scales (and their register bit settings) are:
	// 125 DPS (100), 250 DPS (011), 500 DPS (010), 1000 DPS (001), and 2000 DPS (000). 
    case GFS55_125DPS:
          return 124.87/32768.0; // per data sheet, not exactly 125!?          
    case GFS55_250DPS:
          return  249.75/32768.0;          
    case GFS55_500DPS:
          return  499.5/32768.0;          
    case GFS55_1000DPS:
          return  999.0/32768.0;          
    case GFS55_2000DPS:
          return  1998.0/32768.0;          
  }
  return 0.0f;
}

float getBMX055resAccel() {
  switch (Ascale)
  {
 	// Possible accelerometer scales (and their register bit settings) are:
	// 2 Gs (0011), 4 Gs (0101), 8 Gs (1000), and 16 Gs  (1100). 
        // BMX055 ACC data is signed 12 bit
    case AFS_2G:
          return 2.0/2048.0;          
    case AFS_4G:
          return  4.0/2048.0;          
    case AFS_8G:
          return  8.0/2048.0;          
    case AFS_16G:
          return  16.0/2048.0;          
  }
  return 0.0f;
}

float getBMX055resMag() {
  // magnetometer resolution is 1 microTesla/16 counts or 1/1.6 milliGauss/count
   return 1./1.6;
}

bool readBMX055DataAccel(int16_t *destination)
{
	uint8_t rawData[6];  // x/y/z accel register data stored here
	i2c_readData(BMX055_ACC_ADDRESS, BMX055_ACC_D_X_LSB, rawData, 6);  // Read the six raw data registers into data array
	destination[1] = -((int16_t)((rawData[1] << 8) | rawData[0]) / 8);  // Turn the MSB and LSB into a signed 12-bit value
	destination[0] = ((int16_t)((rawData[3] << 8) | rawData[2]) / 8);  
	destination[2] = ((int16_t)((rawData[5] << 8) | rawData[4]) / 8); 
	return true;
}

bool readBMX055DataGyro(int16_t *destination)
{
	uint8_t rawData[6];  // x/y/z gyro register data stored here
	i2c_readData(BMX055_GYRO_ADDRESS, BMX055_GYRO_RATE_X_LSB, rawData, 6);  // Read the six raw data registers sequentially into data array
	destination[1] = -(int16_t) (((int16_t)rawData[1] << 8) | rawData[0]);   // Turn the MSB and LSB into a signed 16-bit value
	destination[0] = (int16_t) (((int16_t)rawData[3] << 8) | rawData[2]);  
	destination[2] = (int16_t) (((int16_t)rawData[5] << 8) | rawData[4]); 
	return true;
}



bool readBMX055DataMag(int16_t *destination)
{
	int16_t mdata_x = 0, mdata_y = 0, mdata_z = 0; //, temp = 0;
	uint16_t data_r = 0;
	uint8_t rawData[8];  // x/y/z hall magnetic field data, and Hall resistance data
	i2c_readData(BMX055_MAG_ADDRESS, BMX055_MAG_XOUT_LSB, rawData, 8);  // Read the eight raw data registers sequentially into data array
	if(rawData[6] & 0x01) // Check if data ready status bit is set
	{
		mdata_x = (int16_t) (((int16_t)rawData[1] << 8) | rawData[0]) >> 3;  // 13-bit signed integer for x-axis field
		mdata_y = (int16_t) (((int16_t)rawData[3] << 8) | rawData[2]) >> 3;  // 13-bit signed integer for y-axis field
		mdata_z = (int16_t) (((int16_t)rawData[5] << 8) | rawData[4]) >> 1;  // 15-bit signed integer for z-axis field	
		
		data_r = (uint16_t) (((uint16_t)rawData[7] << 8) | rawData[6]) >> 2;  // 14-bit unsigned integer for Hall resistance

		destination[0] = -compensate_BMX055_X(mdata_x, data_r);
		destination[1] = -compensate_BMX055_Y(mdata_y, data_r);
		destination[2] = -compensate_BMX055_Z(mdata_z, data_r);
		
		//auto offset calibration
		if (destination[0] > magMaxX) magMaxX = destination[0]; if (destination[0] < magMinX) magMinX = destination[0]; int16_t magDiffX = (magMaxX + magMinX) / 2; destination[0] -= magDiffX;
		if (destination[1] > magMaxY) magMaxY = destination[1]; if (destination[1] < magMinY) magMinY = destination[1]; int16_t magDiffY = (magMaxY + magMinY) / 2; destination[1] -= magDiffY;
		if (destination[2] > magMaxZ) magMaxZ = destination[2]; if (destination[2] < magMinZ) magMinZ = destination[2]; int16_t magDiffZ = (magMaxZ + magMinZ) / 2; destination[2] -= magDiffZ;
		
//		// calculate temperature compensated 16-bit magnetic fields
//		temp = ((int16_t)(((uint16_t)((((int32_t)dig_xyz1) << 14)/(data_r != 0 ? data_r : dig_xyz1))) - ((uint16_t)0x4000)));
//		destination[0] = ((int16_t)((((int32_t)mdata_x) * ((((((((int32_t)dig_xy2) * ((((int32_t)temp) * ((int32_t)temp)) >> 7)) + (((int32_t)temp) * ((int32_t)(((int16_t)dig_xy1) << 7)))) >> 9) + ((int32_t)0x100000)) * ((int32_t)(((int16_t)dig_x2) + ((int16_t)0xA0)))) >> 12)) >> 13)) + (((int16_t)dig_x1) << 3);

//		//temp = ((int16_t)(((uint16_t)((((int32_t)dig_xyz1) << 14)/(data_r != 0 ? data_r : dig_xyz1))) - ((uint16_t)0x4000)));
//		destination[1] = ((int16_t)((((int32_t)mdata_y) * ((((((((int32_t)dig_xy2) * ((((int32_t)temp) * ((int32_t)temp)) >> 7)) + (((int32_t)temp) * ((int32_t)(((int16_t)dig_xy1) << 7)))) >> 9) +((int32_t)0x100000)) * ((int32_t)(((int16_t)dig_y2) + ((int16_t)0xA0)))) >> 12)) >> 13)) + (((int16_t)dig_y1) << 3);
//		
//		destination[2] = (((((int32_t)(mdata_z - dig_z4)) << 15) - ((((int32_t)dig_z3) * ((int32_t)(((int16_t)data_r) - ((int16_t)dig_xyz1))))>>2))/(dig_z2 + ((int16_t)(((((int32_t)dig_z1) * ((((int16_t)data_r) << 1)))+(1<<15))>>16))));
		
		return true;
	}	
	return false;
}

#define BMM050_INIT_VALUE 				(0)
#define BMM050_FLIP_OVERFLOW_ADCVAL		(-4096)
#define BMM050_HALL_OVERFLOW_ADCVAL		(-16384)
#define BMM050_OVERFLOW_OUTPUT			(-32768)
#define BMM050_NEGATIVE_SATURATION_Z    (-32767)
#define BMM050_POSITIVE_SATURATION_Z    (32767)

int16_t compensate_BMX055_X(int16_t mag_data_x, uint16_t data_r)
{
	int16_t inter_retval = BMM050_INIT_VALUE;
	/* no overflow */
	if (mag_data_x != BMM050_FLIP_OVERFLOW_ADCVAL) {
		if ((data_r != BMM050_INIT_VALUE)
		&& (dig_xyz1 != BMM050_INIT_VALUE)) {
			inter_retval = ((int16_t)(((uint16_t)
			((((int32_t)dig_xyz1) << 14)/
			 (data_r != BMM050_INIT_VALUE ?
			 data_r : dig_xyz1))) -
			((uint16_t)0x4000)));
		} else {
			inter_retval = BMM050_OVERFLOW_OUTPUT;
			return inter_retval;
		}
		inter_retval = ((int16_t)((((int32_t)mag_data_x) *
				((((((((int32_t)dig_xy2) *
				((((int32_t)inter_retval) *
				((int32_t)inter_retval)) >>
				7)) +
			     (((int32_t)inter_retval) *
			      ((int32_t)(((int16_t)dig_xy1)
			      << 7))))
				  >> 9) +
			   ((int32_t)0x100000)) *
			  ((int32_t)(((int16_t)dig_x2) +
			  ((int16_t)0xA0)))) >>
			  12))
			  >> 13)) +
			(((int16_t)dig_x1)
			<< 3);
	} else {
		/* overflow */
		inter_retval = BMM050_OVERFLOW_OUTPUT;
	}
	return inter_retval;
}

int16_t compensate_BMX055_Y(int16_t mag_data_y, uint16_t data_r)
{
	int16_t inter_retval = BMM050_INIT_VALUE;
	 /* no overflow */
	if (mag_data_y != BMM050_FLIP_OVERFLOW_ADCVAL) {
		if ((data_r != BMM050_INIT_VALUE)
		&& (dig_xyz1 != BMM050_INIT_VALUE)) {
			inter_retval = ((int16_t)(((uint16_t)(((
			(int32_t)dig_xyz1)
			<< 14)/
			(data_r != BMM050_INIT_VALUE ?
			 data_r : dig_xyz1))) -
			((uint16_t)0x4000)));
		} else {
			inter_retval = BMM050_OVERFLOW_OUTPUT;
			return inter_retval;
		}
		inter_retval = ((int16_t)((((int32_t)mag_data_y) * ((((((((int32_t)
			dig_xy2) * ((((int32_t) inter_retval) *
			((int32_t)inter_retval)) >>
			7))
			+ (((int32_t)inter_retval) *
			((int32_t)(((int16_t)dig_xy1) <<
			7))))
			>> 9) +
			((int32_t)0x100000)) *
			((int32_t)(((int16_t)dig_y2)
			+ ((int16_t)0xA0))))
			>> 12))
			>> 13)) +
			(((int16_t)dig_y1)
			<< 3);
	} else {
		/* overflow */
		inter_retval = BMM050_OVERFLOW_OUTPUT;
	}
	return inter_retval;
}

int16_t compensate_BMX055_Z(int16_t mag_data_z, uint16_t data_r)
{
	int32_t retval = BMM050_INIT_VALUE;

	if ((mag_data_z != BMM050_HALL_OVERFLOW_ADCVAL)	/* no overflow */
	   ) {
		if ((dig_z2 != BMM050_INIT_VALUE)
		&& (dig_z1 != BMM050_INIT_VALUE)
		&& (data_r != BMM050_INIT_VALUE)
		&& (dig_xyz1 != BMM050_INIT_VALUE)) {
			retval = (((((int32_t)(mag_data_z - dig_z4))
			<< 15) -
			((((int32_t)dig_z3) * ((int32_t)(((int16_t)data_r) -
			((int16_t) dig_xyz1))))
			>> 2))/
			(dig_z2 + ((int16_t)(((((int32_t)
			dig_z1) * ((((int16_t)data_r)
			<< 1)))+
			(1 << 15))
			>> 16))));
		} else {
			retval = BMM050_OVERFLOW_OUTPUT;
			return retval;
		}
		/* saturate result to +/- 2 microTesla */
		if (retval > BMM050_POSITIVE_SATURATION_Z) {
			retval =  BMM050_POSITIVE_SATURATION_Z;
		} else {
			if (retval < BMM050_NEGATIVE_SATURATION_Z)
				retval = BMM050_NEGATIVE_SATURATION_Z;
		}
	} else {
		/* overflow */
		retval = BMM050_OVERFLOW_OUTPUT;
	}
	return (int16_t)retval;
}
