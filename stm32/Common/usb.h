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

#define CMD_NONE 			0x00
#define CMD_VIBRATE			0x01
#define CMD_CALIBRATE		0x02
#define CMD_SYNC			0x03
#define CMD_RAW_DATA		0x04


#define BUTTON_0 0x01
#define BUTTON_1 0x02
#define BUTTON_2 0x04
#define BUTTON_3 0x08

#define SENSOR_ACCEL	0x01
#define SENSOR_GYRO		0x02
#define SENSOR_MAG		0x04

struct USBDataHeader
{
	uint8_t Type; //source & data
	uint16_t Sequence; //source & data
	uint8_t Crc8; //source & data
};

struct USBPositionData
{
	uint16_t X1; //elapsed time since sync1 & 2
	uint16_t Y1;
	uint16_t X2;
	uint16_t Y2;
	uint16_t X3;
	uint16_t Y3;
	uint16_t X4;
	uint16_t Y4;
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
	uint8_t State; //1 for enable, 0 for disable (set by driver)
	int16_t Accel[3];
	int16_t Gyro[3];
	int16_t Mag[3];
};

struct USBTriggerData
{	
	uint16_t Digital;
	struct USBAxisData Analog[2];
};

struct USBCalibrationData
{
	uint8_t SensorMask;	
	int16_t OffsetAccel[3];
	int16_t OffsetGyro[3];
	int16_t OffsetMag[3];
};

struct USBVibrationData
{
	uint32_t Axis;
	uint16_t Duration;
};

struct USBStatusData
{
	uint8_t CalibrationMask;
	
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

