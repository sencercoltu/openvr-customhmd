#include "windows.h"
#define ESCAPI_DEFINITIONS_ONLY
#include "escapi.h"


#define MAXDEVICES 16

extern struct SimpleCapParams gParams[];
//extern int gDoCapture[];
//extern int gOptions[];

extern HRESULT InitDevice(int device);
extern void CleanupDevice(int device);
extern int CountCaptureDevices();
extern void GetCaptureDeviceName(int deviceno, char * namebuffer, int bufferlength);
extern void CheckForFail(int device);
extern int GetErrorCode(int device);
extern int GetErrorLine(int device);
extern float GetProperty(int device, int prop);
extern int GetPropertyAuto(int device, int prop);
extern int SetProperty(int device, int prop, float value, int autoval);


void getCaptureDeviceName(unsigned int deviceno, char *namebuffer, int bufferlength)
{
	if (deviceno > MAXDEVICES)
		return;

	GetCaptureDeviceName(deviceno, namebuffer, bufferlength);
}

int ESCAPIDLLVersion()
{
	return 0x200; // due to mess up, earlier programs check for exact version; this needs to stay constant
}

int ESCAPIVersion()
{
	return 0x301; // ...and let's hope this one works better
}

int countCaptureDevices()
{
	int c = CountCaptureDevices();
	return c;
}

void initCOM()
{
	CoInitialize(NULL);
}

int initCapture(unsigned int deviceno, struct SimpleCapParams *aParams)
{
	if (deviceno > MAXDEVICES)
		return 0;
	if (aParams == NULL || aParams->mHeight <= 0 || aParams->mWidth <= 0 || aParams->mTargetBuf == 0)
		return 0;
	//gDoCapture[deviceno] = 0;
	gParams[deviceno] = *aParams;
	//gOptions[deviceno] = 0;
	if (FAILED(InitDevice(deviceno))) return 0;
	return 1;
}

void deinitCapture(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return;
	CleanupDevice(deviceno);
}

void doCapture(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return;
	CheckForFail(deviceno);
	//gDoCapture[deviceno] = -1;
}

//int isCaptureDone(unsigned int deviceno)
//{
//	if (deviceno > MAXDEVICES)
//		return 0;
//	CheckForFail(deviceno);
//	if (gDoCapture[deviceno] == 1)
//		return 1;
//	return 0;
//}

int getCaptureErrorLine(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetErrorLine(deviceno);
}

int getCaptureErrorCode(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetErrorCode(deviceno);
}

float getCapturePropertyValue(unsigned int deviceno, int prop)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetProperty(deviceno, prop);
}

int getCapturePropertyAuto(unsigned int deviceno, int prop)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetPropertyAuto(deviceno, prop);
}

int setCaptureProperty(unsigned int deviceno, int prop, float value, int autoval)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return SetProperty(deviceno, prop, value, autoval);
}

//int initCaptureWithOptions(unsigned int deviceno, struct SimpleCapParams *aParams, unsigned int aOptions)
//{
//	if (deviceno > MAXDEVICES)
//		return 0;
//	if (aParams == NULL || aParams->mHeight <= 0 || aParams->mWidth <= 0 || aParams->mTargetBuf == 0)
//		return 0;
//	if ((aOptions & CAPTURE_OPTIONS_MASK) != aOptions)
//		return 0;
//	gDoCapture[deviceno] = 0;
//	gParams[deviceno] = *aParams;
//	//gOptions[deviceno] = aOptions;
//	if (FAILED(InitDevice(deviceno))) return 0;
//	return 1;
//}

