#pragma once

#ifndef Common_H
#define Common_H

#include <Windows.h>
#include <openvr_driver.h>
#include <vector>
#include <memory>
#include <string>

//#define TRACE(a) OutputDebugString(a"\n")

#define HMD_DLL_EXPORT extern "C" __declspec(dllexport)
#define HMD_DLL_IMPORT extern "C" __declspec(dllimport)

#define HMD_MODE_AMD 


struct MonitorData
{
	WCHAR DisplayName[CCHDEVICENAME];
	bool HMD_FOUND;
	int HMD_POSX;
	int HMD_POSY;
	int HMD_WIDTH;
	int HMD_HEIGHT;
	float HMD_FREQ;
	float HMD_ASPECT;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

vr::HmdQuaternion_t HmdQuaternion_Init(double w, double x, double y, double z);
void HmdMatrix_SetIdentity(vr::HmdMatrix34_t *pMatrix);

void EnableAMDHD3D();


#endif // Common_H