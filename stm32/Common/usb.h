#include <stdint.h>

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

#define HMD_SOURCE 0x00
#define LEFTCTL_SOURCE 0x01
#define RIGHTCTL_SOURCE 0x02

#define ROTATION_DATA 0x10
#define POSITION_DATA 0x20
#define TRIGGER_DATA 0x40
#define COMMAND_DATA 0x80

#define CMD_NONE 		0x00
#define CMD_VIBRATE		0x01
#define CMD_CALIBRATE	0x02


struct USBDataHeader
{
	uint8_t Type; //source & data
	uint16_t Sequence; //source & data
	uint8_t Crc8; //source & data
};

struct USBPositionData
{
	struct USBDataHeader Header;
	uint16_t X1;
	uint16_t Y1;
	uint16_t X2;
	uint16_t Y2;
	uint16_t X3;
	uint16_t Y3;
	uint16_t X4;
	uint16_t Y4;
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

struct USBTriggerData
{	
	uint16_t Digital;
	struct USBAxisData Analog[2];
};


struct USBVibrationData
{
	uint32_t Axis;
	uint16_t Duration;
};

union CommandData
{
	struct USBVibrationData Vibration;
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

struct USBDataCache
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

#pragma pack(pop)
