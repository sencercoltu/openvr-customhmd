#pragma once

#define WIN32_LEAN_AND_MEAN

#ifndef Common_H
#define Common_H

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "resource.h"
#include <openvr_driver.h>
#include <vector>
#include <memory>
#include <string>
#include <locale>
#include <codecvt>
#include <HighLevelMonitorConfigurationAPI.h>
#include "escapi/capturedevice.h"
#include "..\stm32\Common\usb.h"
#include "ShMem.h"
#include "LiquidVR.h"

#define SAFE_RELEASE(a) if(a) a->Release(); a = nullptr; 
#define SAFE_CLOSE(a) if(a) CloseHandle((HANDLE)a); a = nullptr; 
#define SAFE_FREE(a) if(a) free((void *)a); a = nullptr; 

extern bool IsD2DConnected(uint16_t edid);

//#define _LOG(f, ...) m_pLog->Log(f, __VA_ARGS__)

#define HMD_DLL_EXPORT extern "C" __declspec(dllexport)
#define HMD_DLL_IMPORT extern "C" __declspec(dllimport)

#define VKD(a) ((GetAsyncKeyState(a) & 0x8000))

using namespace vr;

class CDriverLog
{
private:
	vr::IVRDriverLog *s_pLogFile;
public:
	CDriverLog()
	{
		s_pLogFile = nullptr;
	}

	~CDriverLog()
	{
		s_pLogFile = nullptr;
	}

	bool InitDriverLog(vr::IVRDriverLog *pDriverLog)
	{
		if (s_pLogFile)
			return false;
		s_pLogFile = pDriverLog;
		return s_pLogFile != nullptr;
	}

	void CleanupDriverLog()
	{
		s_pLogFile = nullptr;
	}

	void DriverLog(const char *pMsgFormat, ...)
	{
		va_list args;
		va_start(args, pMsgFormat);
		DriverLogVarArgs(pMsgFormat, args);
		va_end(args);
	}

private:

	void DriverLogVarArgs(const char *pMsgFormat, va_list args)
	{
		char buf[8192];
		vsprintf_s(buf, pMsgFormat, args);
		strcat_s(buf, "\n");
		if (s_pLogFile)
			s_pLogFile->Log(buf);
	}
};
//
//#pragma pack(push)
//#pragma pack(1)
//
//struct USBPositionData
//{
//	float x;
//	float y;
//	float z;
//};
//
//struct USBRotationData
//{
//	float w;
//	float x;
//	float y;
//	float z;
//};
//
//#define CUSTOM_HID_EPOUT_SIZE 32
//#define CUSTOM_HID_EPIN_SIZE 32
//
//#define HMD_SOURCE 0x00
//#define LEFTCTL_SOURCE 0x01
//#define RIGHTCTL_SOURCE 0x02
//
//#define ROTPOS_DATA 0x10
//#define TRIGGER_DATA 0x20
//
//#define HMD_ROTPOSDATA (HMD_SOURCE | ROTPOS_DATA)
//
//struct USBDataHeader
//{
//	uint8_t Type; //source & data
//	uint8_t Sequence; //source & data
//	uint8_t Crc8; //simple crc, only for rf, ignored on usb
//};
//
//struct USBRotPosData
//{
//	USBDataHeader Header;
//	USBRotationData Rotation;
//	USBPositionData Position;
//};
//
//struct USBAxisData
//{
//	float x;
//	float y;
//};
//
//struct USBTriggerData
//{
//	USBDataHeader Header;
//	uint16_t Digital;
//	USBAxisData Analog[2];
//};
//
//union USBData
//{
//	USBRotPosData RotPos;
//	USBTriggerData Trigger;
//};
//
//struct USBOutputPacket
//{
//	USBData Data;
//	uint8_t Reserved[CUSTOM_HID_EPOUT_SIZE - sizeof(USBData)];
//};
//
//#pragma pack(pop)


struct TrackerData
{
	HANDLE hPoseLock;
	bool PoseUpdated;
	DriverPose_t Pose;
};

struct CameraData 
{
	char Model[128];
	HANDLE hLock;			
	GUID MediaFormat;
	CCaptureDevice::CaptureOptions Options;
	CCaptureDevice *pCaptureDevice;
	ECameraVideoStreamFormat StreamFormat;
	CameraVideoStreamFrame_t SetupFrame; 
	CameraVideoStreamFrame_t *pFrameBuffer;	
	ICameraVideoSinkCallback *pfCallback;	
	DWORD CallbackCount;
	DWORD StartTime;
	DWORD LastFrameTime;
	void Destroy()
	{
		if (pCaptureDevice)
		{
			pCaptureDevice->Release();
			pCaptureDevice = nullptr;
		}

		if (hLock != nullptr)
		{
			CloseHandle(hLock); 
			hLock = nullptr;
		}
		Options.Destroy();
		ZeroMemory(this, sizeof(*this));
	}

};

class CTrackedHMD;

struct HMDData : TrackerData
{
	CTrackedHMD *pHMDDriver;
	WCHAR DisplayName[CCHDEVICENAME];
	WCHAR Model[128];		
	bool IsConnected;
	bool Windowed;
	bool DirectMode;
	bool FakePackDetected;
	int PosX;
	int PosY;
	int ScreenWidth;
	int ScreenHeight;
	int EyeWidth;	
	//int EyeTexWidth;
	//int EyeTexHeight;
	float Frequency;
	float AspectRatio;	
	float SuperSample;		
	USBDataCache LastState;	
	int RemoteDisplayPort;
	char RemoteDisplayHost[128];
};

struct ControllerData : TrackerData
{
	WCHAR DisplayName[CCHDEVICENAME];
	WCHAR Model[128];	
	VRControllerState_t State;
	HmdVector3d_t Euler;
	USBDataCache LastState;
};

#endif // Common_H