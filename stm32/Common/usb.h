#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	
#ifndef CUSTOM_HID_EPIN_SIZE
#define CUSTOM_HID_EPIN_SIZE 32
#endif //CUSTOM_HID_EPIN_SIZE

#ifndef CUSTOM_HID_EPOUT_SIZE
#define CUSTOM_HID_EPOUT_SIZE 32
#endif //CUSTOM_HID_EPOUT_SIZE

#ifndef USB_CUSTOM_HID_DESC_SIZ
#define USB_CUSTOM_HID_DESC_SIZ 34
#endif //USB_CUSTOM_HID_DESC_SIZ

#pragma pack(push)
#pragma pack(1)

#define HMD_SOURCE 			0x00
#define LEFTCTL_SOURCE 		0x01
#define RIGHTCTL_SOURCE 	0x02
#define BASESTATION_SOURCE 	0x03
#define LIGHTHOUSE1_SOURCE 	0x04
#define LIGHTHOUSE2_SOURCE 	0x05
#define MAX_SOURCE 			0x06
#define INVALID_SOURCE 		0x0F

#define ROTATION_DATA 0x10
#define POSITION_DATA 0x20
#define TRIGGER_DATA 0x40
#define COMMAND_DATA 0x80

#define CALIB_SET	(uint8_t)0x00
#define CALIB_GET	(uint8_t)0x01

#define CMD_NONE 			(uint8_t)0x00
#define CMD_VIBRATE			(uint8_t)0x01
#define CMD_CALIBRATE		(uint8_t)0x02
#define CMD_SYNC			(uint8_t)0x03
#define CMD_RAW_DATA		(uint8_t)0x04
#define CMD_STATUS			(uint8_t)0x05


#define BUTTON_0 0x01
#define BUTTON_1 0x02
#define BUTTON_2 0x04
#define BUTTON_3 0x08
#define BUTTON_4 0x10
#define BUTTON_5 0x20
#define BUTTON_6 0x40
#define BUTTON_7 0x80

#define SENSOR_NONE		(uint8_t)0x00
#define SENSOR_ACCEL	(uint8_t)0x01
#define SENSOR_GYRO		(uint8_t)0x02
#define SENSOR_MAG		(uint8_t)0x03
//#define SENSOR_POSACCEL	0x08

struct USBDataHeader
{
	uint8_t Type; //source & data
	uint16_t Sequence; //source & data
	uint8_t Crc8; //source & data
};

//struct USBPositionData
//{
//	uint16_t X1; //elapsed time since sync1 & 2
//	uint16_t Y1;
//	uint16_t X2;
//	uint16_t Y2;
//	uint16_t X3;
//	uint16_t Y3;
//	uint16_t X4;
//	uint16_t Y4;	
//};

struct USBPositionData
{
	float Position[3];
};


struct USBSyncData
{	
	uint64_t SyncTime;
};

struct USBRotationData
{		
	float w;
	float x;
	float y;
	float z;
};

struct USBAxisData
{
	float x;
	float y;
};

struct USBRawData
{
	uint8_t State; //1 for enable, 0 for disable (set by driver), 2 for filtered
	int16_t Accel[3];
	int16_t Gyro[3];
	int16_t Mag[3];	
};

struct USBTriggerData
{	
	uint16_t Digital;
	struct USBAxisData Analog[2];
};

// 2/65535 ( 0.00003052 to 2.0)
//#define TO_CALIB_SCALE(x) ((uint16_t)((float)x * 32768.0f))
//#define FROM_CALIB_SCALE(x) ((float)((float)x / 32768.0f))

#define VALID_EEPROM_MAGIC	0x3DD3D33D

struct USBCalibrationData
{
	uint8_t Command;	
	uint8_t Sensor;	
	int16_t RawMax[3];
	int16_t RawMin[3];	
	uint16_t Reserved;
	//int16_t OffsetAccel[3];	
	//uint16_t ScaleAccel[3]; //sil bunu
	//int16_t OffsetGyro[3];
	//int16_t OffsetMag[3];	
};

struct EepromData
{
	uint32_t Magic;
	struct USBCalibrationData Accel;
	struct USBCalibrationData Gyro;
	struct USBCalibrationData Mag;
	uint16_t Reserved;
};


struct USBVibrationData
{
	uint32_t Axis;
	uint16_t Duration;
};

struct USBStatusData
{
	uint32_t HeartBeat;
	
};

union CommandData
{
	struct USBVibrationData Vibration;
	struct USBSyncData Sync;
	struct USBCalibrationData Calibration;
	struct USBRawData Raw;
	struct USBStatusData Status;	
};

struct USBCommandData
{	
	uint8_t Command;
	union CommandData Data;	
};

union USBData
{	
	struct USBPositionData Position;
	struct USBRotationData Rotation;
	struct USBTriggerData Trigger;
	struct USBCommandData Command;		
};

struct USBDataCache //used at ps side/server only
{
	struct USBPositionData Position;
	struct USBRotationData Rotation;
	struct USBTriggerData Trigger;
};

struct USBPacket
{
	struct USBDataHeader Header;
	//union USBData Data;
	union 
	{	
		struct USBPositionData Position;
		struct USBRotationData Rotation;
		struct USBTriggerData Trigger;
		struct USBCommandData Command;
	};

	uint8_t Reserved[CUSTOM_HID_EPOUT_SIZE - (sizeof(union USBData) + sizeof(struct USBDataHeader))];
};

void SetPacketCrc(struct USBPacket *pPacket);
uint8_t GetPacketCrc(struct USBPacket *pPacket);
uint8_t CheckPacketCrc(struct USBPacket *pPacket);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

