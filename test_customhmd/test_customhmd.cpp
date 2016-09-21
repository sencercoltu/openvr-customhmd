/*******************************************************
 Windows HID simplification

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009
 
 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.
********************************************************/

#define _USE_MATH_DEFINES

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>
#include <math.h>
#include "..\driver_customhmd\hidapi.h"
#include "..\stm32\Common\usb.h"
#include <string>

// Headers needed for sleeping.
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif




extern USBPacket *pUSBCommandPacket;
extern USBPacket USBDataPacket;

struct HmdVector3d_t
{
	double v[3];
};

struct Quaternion
{
	double w, x, y, z;
};

Quaternion FromEuler(float *v)
{
	Quaternion q;
	// Assuming the angles are in radians.
	double c1 = cos(v[0] / 2);
	double s1 = sin(v[0] / 2);
	double c2 = cos(v[1] / 2);
	double s2 = sin(v[1] / 2);
	double c3 = cos(v[2] / 2);
	double s3 = sin(v[2] / 2);
	double c1c2 = c1*c2;
	double s1s2 = s1*s2;
	q.w = c1c2*c3 - s1s2*s3;
	q.x = c1c2*s3 + s1s2*c3;
	q.y = s1*c2*c3 + c1*s2*s3;
	q.z = c1*s2*c3 - s1*c2*s3;
	return q;
}

#define DEG(a) (a*180.0/M_PI)

#define WORD_TO_BINARY_PATTERN "%c-%c-%c-%c-%c-%c-%c-%c-%c-%c-%c-%c-%c-%c-%c-%c"
#define WORD_TO_BINARY(WORD)  \
  (WORD & 0x8000 ? '1' : '0'), \
  (WORD & 0x4000 ? '1' : '0'), \
  (WORD & 0x2000 ? '1' : '0'), \
  (WORD & 0x1000 ? '1' : '0'), \
  (WORD & 0x0800 ? '1' : '0'), \
  (WORD & 0x0400 ? '1' : '0'), \
  (WORD & 0x0200 ? '1' : '0'), \
  (WORD & 0x0100 ? '1' : '0'), \
  (WORD & 0x0080 ? '1' : '0'), \
  (WORD & 0x0040 ? '1' : '0'), \
  (WORD & 0x0020 ? '1' : '0'), \
  (WORD & 0x0010 ? '1' : '0'), \
  (WORD & 0x0008 ? '1' : '0'), \
  (WORD & 0x0004 ? '1' : '0'), \
  (WORD & 0x0002 ? '1' : '0'), \
  (WORD & 0x0001 ? '1' : '0') 


int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[sizeof(USBPacket) + 1];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;

	
	if (hid_init())
		return -1;


	/*struct hid_device_info *devs, *cur_dev;

	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;	
	while (cur_dev) {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n",  cur_dev->interface_number);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);*/

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;
	

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	handle = hid_open(0x1974, 0x0001, NULL);
	if (!handle) {
		printf("unable to open device\n");
 		return 1;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	else
		printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	else
		printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	else
		printf("Serial Number String: (%d) %ls\n", wstr[0], wstr);
	

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);
	

	struct LocalCalibData  
	{
		uint8_t State; 
		float Accel[3];
		float MaxAccel[3];
		float MinAccel[3];
		float Gyro[3];
		float Mag[3];
		float MaxMag[3];
		float MinMag[3];
	} calib;
	calib = { 0 };
	
	int rawState = 0;

	auto xxx = (USBPacket*)buf;
	auto yyy = (USBPacket*)(buf+1);
	uint8_t lastType = 0;
	int count = 0;
	while (true)
	{		
		

		//res = hid_write(handle, buf, sizeof(buf));
		//printf("Written %d bytes\n", res);
		if (handle) 		
			res = hid_read_timeout(handle, buf, sizeof(buf), 10);
		if (res > 0)
		{
			if (CheckPacketCrc(xxx))
			{
				auto sequence = xxx->Header.Sequence;
				auto source = xxx->Header.Type & 0x0F;
				auto type = xxx->Header.Type & 0xF0;
				
				//if (type != POSITION_DATA) continue;

				switch (source)
				{
					case HMD_SOURCE:
						printf("HMD");
						break;
					case LEFTCTL_SOURCE:
						printf("LCTL");
						break;
					case RIGHTCTL_SOURCE:
						printf("RCTL");
						break;
					case BASESTATION_SOURCE:
						printf("BASESTATION_SOURCE");
						break;
					default:
						printf("!!UNKNOWN_SOURCE!!");
						break;
				}

				printf("\t");

				switch (type)
				{
					case ROTATION_DATA:
						printf("ROTATION_DATA\t");
						printf("w:%f x:%f y:%f z:%f", xxx->Rotation.w, xxx->Rotation.x, xxx->Rotation.y, xxx->Rotation.z);
						break;
					case POSITION_DATA:
						printf("POSITION_DATA\t");
						printf("x:%f y:%f z:%f", xxx->Position.Position[0], xxx->Position.Position[1], xxx->Position.Position[2]);
						break;
					case TRIGGER_DATA:
						printf("TRIGGER_DATA\t");
						printf("Digital: " WORD_TO_BINARY_PATTERN " A0x: %f A0y: %f A1x: %f A1y: %f", WORD_TO_BINARY(xxx->Trigger.Digital), xxx->Trigger.Analog[0].x, xxx->Trigger.Analog[0].y, xxx->Trigger.Analog[1].x, xxx->Trigger.Analog[1].y);
						break;
					case COMMAND_DATA:
						printf("COMMAND_DATA\t");
						switch (xxx->Command.Command)
						{
							case CMD_RAW_DATA:
								//calc averages
								if (!calib.State)
								{
									for (int i = 0; i < 3; i++)
									{										
										calib.Accel[i] = xxx->Command.Data.Raw.Accel[i];
										calib.MaxAccel[i] = FLT_MIN;
										calib.MinAccel[i] = FLT_MAX;
										calib.Gyro[i] = xxx->Command.Data.Raw.Gyro[i];
										calib.Mag[i] = xxx->Command.Data.Raw.Mag[i];										
										calib.MaxMag[i] = FLT_MIN;
										calib.MinMag[i] = FLT_MAX;
									}									

									calib.State = 1;
								}
								else
								{
									for (int i = 0; i < 3; i++)
									{										
										calib.Accel[i] = (calib.Accel[i] * 0.95f) + ((float)xxx->Command.Data.Raw.Accel[i] * 0.05f); 
										if (calib.Accel[i] > calib.MaxAccel[i]) calib.MaxAccel[i] = calib.Accel[i];
										if (calib.Accel[i] < calib.MinAccel[i]) calib.MinAccel[i] = calib.Accel[i];
										calib.Gyro[i] = (calib.Gyro[i] * 0.95f) + ((float)xxx->Command.Data.Raw.Gyro[i] * 0.05f); 
										calib.Mag[i] = (calib.Mag[i] * 0.95f) + ((float)xxx->Command.Data.Raw.Mag[i] * 0.05f);
										if (calib.Mag[i] > calib.MaxMag[i]) calib.MaxMag[i] = calib.Mag[i];
										if (calib.Mag[i] < calib.MinMag[i]) calib.MinMag[i] = calib.Mag[i];
									}
								}

								//printf("AX: %05d AY: %05d AZ: %05d GX: %05d GY: %05d GZ: %05d MX: %05d MY: %05d MZ: %05d\n", 
								//	xxx->Command.Data.Raw.Accel[0], xxx->Command.Data.Raw.Accel[1], xxx->Command.Data.Raw.Accel[2],
								//	xxx->Command.Data.Raw.Gyro[0], xxx->Command.Data.Raw.Gyro[1], xxx->Command.Data.Raw.Gyro[2],
								//	xxx->Command.Data.Raw.Mag[0], xxx->Command.Data.Raw.Mag[1], xxx->Command.Data.Raw.Mag[2]);

								printf("\nAAX: %05d AAY: %05d AAZ: %05d\nAGX: %05d AGY: %05d AGZ: %05d\nAMX: %05d AMY: %05d AMZ: %05d\n",
									(int)calib.Accel[0], (int)calib.Accel[1], (int)calib.Accel[2],
									(int)calib.Gyro[0], (int)calib.Gyro[1], (int)calib.Gyro[2],
									(int)calib.Mag[0], (int)calib.Mag[1], (int)calib.Mag[2]);
								printf("LAX:(%05d / %05d) LAY:(%05d / %05d) LAZ:(%05d / %05d)\nLMX:(%05d / %05d) LMY:(%05d / %05d) LMZ:(%05d / %05d)\n",
									(int)calib.MinAccel[0], (int)calib.MaxAccel[0], (int)calib.MinAccel[1], (int)calib.MaxAccel[1], (int)calib.MinAccel[2], (int)calib.MaxAccel[2],
									(int)calib.MinMag[0], (int)calib.MaxMag[0], (int)calib.MinMag[1], (int)calib.MaxMag[1], (int)calib.MinMag[2], (int)calib.MaxMag[2]);

								break;
						}
						break;
					default: 
						printf("!!UNKONWN_DATA!!\t");
						break;
				}


				printf("\n");

				//printf("b1:%d b2:%d jx:%012.06f jy:%012.06f ax:%012.06f ay:%012.06f az:%012.06f \n", xxx->Digital[0], xxx->Digital[1], xxx->Analog[0], xxx->Analog[1], xxx->Analog[2], xxx->Analog[3], xxx->Analog[4]);
			}
		}
		else if (res < 0)
		{
			hid_close(handle);
			handle = nullptr;
			handle = hid_open(0x1974, 0x0001, NULL);
		}

		if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000))
		{
			ZeroMemory(&calib, sizeof(calib));
		}
		if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) && ((GetAsyncKeyState(VK_RCONTROL) & 0x8000)))
		{
			printf("Sending raw data cmd\n");
			memset(buf, 0, sizeof(buf));
			yyy->Header.Sequence = (uint16_t)(GetTickCount() / 1000);
			yyy->Header.Type = COMMAND_DATA | HMD_SOURCE;
			yyy->Header.Crc8 = 0;
			yyy->Command.Command = CMD_RAW_DATA;
			rawState++;
			yyy->Command.Data.Raw.State = (rawState % 2);
			SetPacketCrc(yyy);
			hid_write(handle, buf, sizeof(buf));
			//Sleep(500);
		}
		if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) && ((GetAsyncKeyState(VK_LSHIFT) & 0x8000)))
		{
			printf("Sending calib data cmd\n");
			memset(buf, 0, sizeof(buf));
			yyy->Header.Sequence = (uint16_t)(GetTickCount() / 1000);
			yyy->Header.Type = COMMAND_DATA | HMD_SOURCE;
			yyy->Header.Crc8 = 0;
			yyy->Command.Command = CMD_CALIBRATE;
			yyy->Command.Data.Calibration.SensorMask = SENSOR_MAG | SENSOR_ACCEL | SENSOR_GYRO;
			yyy->Command.Data.Calibration.OffsetAccel[0] = 0;  //53;
			yyy->Command.Data.Calibration.OffsetAccel[1] = 0;  //-16;
			yyy->Command.Data.Calibration.OffsetAccel[2] = 0;  //158;
			yyy->Command.Data.Calibration.ScaleAccel[0] = 1; // TO_CALIB_SCALE(0.984142239);
			yyy->Command.Data.Calibration.ScaleAccel[1] = 1; // TO_CALIB_SCALE(0.97803247);
			yyy->Command.Data.Calibration.ScaleAccel[2] = 1; // TO_CALIB_SCALE(1);
			yyy->Command.Data.Calibration.OffsetGyro[0] = 0;
			yyy->Command.Data.Calibration.OffsetGyro[1] = -7;
			yyy->Command.Data.Calibration.OffsetGyro[2] = -5;
			yyy->Command.Data.Calibration.OffsetMag[0] = +20;
			yyy->Command.Data.Calibration.OffsetMag[1] = -50;
			yyy->Command.Data.Calibration.OffsetMag[2] = 187;
			SetPacketCrc(yyy);
			hid_write(handle, buf, sizeof(buf));
		}
	}
	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause");
#endif

	return 0;
}
