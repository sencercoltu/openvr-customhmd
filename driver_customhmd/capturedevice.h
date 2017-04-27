#pragma once

#ifndef CaptureDevice_h
#define CaptureDevice_h

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <shlwapi.h>
#include <objbase.h>
#include <dshow.h>
#include <dmo.h>
#include <wmcodecdsp.h>
//#include <math.h>
#include <string>

class CCaptureDevice : public IMFSourceReaderCallback
{
public:

	typedef void(*pfConverterFunction)(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride);
	typedef void(*pfCameraFrameCallback)(CCaptureDevice *pDevice, void *pUserData);

	struct CaptureOptions
	{
		friend class CCaptureDevice;
	public:
		bool Setup()
		{
			Initialized = false;			
			BufferSize = 0;
			if (MediaFormat == MFVideoFormat_NV12)
			{
				int frame_pixels = Width * Height;
				BufferSize = (frame_pixels * 3) / 2;
			}
			else if (MediaFormat == MFVideoFormat_RGB32)
			{
				int frame_pixels = Width * Height;
				BufferSize = frame_pixels * 4;
			}
			else if (MediaFormat == MFVideoFormat_RGB24)
			{
				int frame_pixels = Width * Height;
				BufferSize = frame_pixels * 3;
			}
			

			if (BufferSize == 0)
				return false;
			Initialized = true;
			return true;
		}

		void Destroy()
		{
			Initialized = false;
		}

		std::string Name;
		unsigned int Width;
		unsigned int Height;
		unsigned int BufferSize;
		GUID MediaFormat;
		pfCameraFrameCallback pfCallback;
		pfConverterFunction pfConverter;
		void *pUserData;
		void *pCaptureBuffer;
	private:
		bool Initialized;
	};

	typedef enum {
		Stopped,
		Started,
		Paused
	} MediaStatus;

	typedef struct 
	{
		GUID SourceFormat;
		GUID DestinationFormat;
		pfConverterFunction Convert;
	} ConverterDef;

	MediaStatus m_Status;


	static CCaptureDevice *CCaptureDevice::GetCaptureDevice(CaptureOptions *pOptions);	

	STDMETHODIMP QueryInterface(REFIID aRiid, void** aPpv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP OnReadSample(HRESULT aStatus, DWORD aStreamIndex, DWORD aStreamFlags, LONGLONG aTimestamp, IMFSample *aSample);
	STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *);
	STDMETHODIMP OnFlush(DWORD);

	bool Start();
	bool Stop();
	bool Pause();
	bool Resume();

	static void NullConverter(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride);
	static void YUY2toNV12(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride);
	static void YUY2toRGB24(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride);
	static void RGB24toNV12(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride);
	static pfConverterFunction GetConverter(GUID &from, GUID&to);
private:
	
	CCaptureDevice(IMFActivate *pDevice, CaptureOptions *pOptions);
	~CCaptureDevice();

	CRITICAL_SECTION		m_CritSec;
	pfConverterFunction		m_Convert;
	long                    m_RefCount;        
	IMFSourceReader         *m_pReader;
	IMFMediaSource			*m_pSource;
	LONG                    m_SourceStride;
	LONG                    m_DestinationStride;
	CaptureOptions			*m_pOptions;
	IMFActivate				*m_pDevice;
	DWORD					m_dwInputID, m_dwOutputID;
};

#endif //CaptureDevice_h