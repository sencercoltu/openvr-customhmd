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

//#define TRACE(a) OutputDebugString(a"\n")

#define HMD_DLL_EXPORT extern "C" __declspec(dllexport)
#define HMD_DLL_IMPORT extern "C" __declspec(dllimport)

struct HMDLog
{
public:
	HMDLog(vr::IDriverLog *pLogger) 
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
		_logger->Log(szMessage);
	}
private:
	vr::IDriverLog *_logger; 
};

struct HMDData
{
	WCHAR DisplayName[CCHDEVICENAME];
	WCHAR Model[128];
	WCHAR Port[32];	
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
	HMDLog *Logger;
	float PIDValue;
	HANDLE hPoseLock;
	bool PoseUpdated;
	vr::DriverPose_t Pose;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

class CDummyLog : public vr::IDriverLog
{
public:
	virtual void Log(const char *pchLogMessage);
};

#endif // Common_H