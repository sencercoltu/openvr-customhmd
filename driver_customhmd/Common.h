#pragma once

#ifndef Common_H
#define Common_H

#include <Windows.h>
#include <openvr_driver.h>
#include <vector>
#include <memory>
#include <string>
#include <locale>
#include <codecvt>
#include <HighLevelMonitorConfigurationAPI.h>
#include "escapi/capturedevice.h"
#include "..\stm32\Common\usb.h"

#include "LiquidVR.h"

extern bool IsD2DConnected(uint16_t edid);

#define _LOG(f, ...) m_pLog->Log(f, __VA_ARGS__)

#define HMD_DLL_EXPORT extern "C" __declspec(dllexport)
#define HMD_DLL_IMPORT extern "C" __declspec(dllimport)

#define VKD(a) ((GetAsyncKeyState(a) & 0x8000))

using namespace vr;

struct CDriverLog
{
public:
	CDriverLog(IDriverLog *pLogger)
	{
		_logger = pLogger;
	}
	void Log(const char *pFmt, ...) {
		if (!_logger)
			return;
		char szMessage[4096];
		va_list argptr;
		va_start(argptr, pFmt);
		vsprintf_s(szMessage, pFmt, argptr);
		va_end(argptr);		
		strcat_s(szMessage, "\n");
		_logger->Log(szMessage);
	}
	void Log(const wchar_t *pFmt, ...) {
		if (!_logger)
			return;
		std::wstring wchar_string(pFmt);
		const std::string basic_string(wchar_string.begin(), wchar_string.end());
		char szMessage[4096];
		va_list argptr;
		va_start(argptr, pFmt);
		vsprintf_s(szMessage, basic_string.c_str(), argptr);
		va_end(argptr);
		strcat_s(szMessage, "\n");
		_logger->Log(szMessage);
	}
private:
	IDriverLog *_logger; 
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

struct HMDData : TrackerData
{
	WCHAR DisplayName[CCHDEVICENAME];
	WCHAR Model[128];
	bool IsConnected;
	bool DirectMode;
	bool FakePackDetected;
	int PosX;
	int PosY;
	int ScreenWidth;
	int ScreenHeight;
	float Frequency;
	float AspectRatio;	
	float SuperSample;
	CDriverLog *Logger;
	float IPDValue;
	USBDataCache LastState;
};

struct ControllerData : TrackerData
{
	WCHAR DisplayName[CCHDEVICENAME];
	WCHAR Model[128];	
	VRControllerState_t State;
	HmdVector3d_t Euler;
	USBDataCache LastState;
};

void CreateDefaultSettings();
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

class CDummyLog : public IDriverLog
{
public:
	void Log(const char *pchLogMessage) override;
};

#endif // Common_H