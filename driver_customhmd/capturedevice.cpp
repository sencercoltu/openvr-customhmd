#include "capturedevice.h"
#include "videobufferlock.h"

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")
#pragma comment(lib, "strmiids.lib")

CCaptureDevice::ConverterDef ConverterFunctions[] = 
{	
	{ MFVideoFormat_YUY2, MFVideoFormat_NV12, CCaptureDevice::YUY2toNV12 },
	{ MFVideoFormat_YUY2, MFVideoFormat_RGB24, CCaptureDevice::YUY2toRGB24 },
	{ MFVideoFormat_RGB24, MFVideoFormat_NV12, CCaptureDevice::RGB24toNV12 }
};

#define EXITIFFAILED(h) if (!SUCCEEDED(h)) goto exitFail

CCaptureDevice *CCaptureDevice::GetCaptureDevice(CaptureOptions *pOptions)
{
	if (!pOptions)
		return nullptr;

	if (pOptions->Name.empty())
		return nullptr;

	std::wstring wName(pOptions->Name.begin(), pOptions->Name.end());
	WCHAR *pName = 0;
	UINT32 nameLen = 255;
	IMFActivate *pDevice = nullptr;

	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	EXITIFFAILED(hr);

	hr = MFStartup(MF_VERSION);
	EXITIFFAILED(hr);

	IMFAttributes *pAttributes = nullptr;
	hr = MFCreateAttributes(&pAttributes, 1);
	EXITIFFAILED(hr);

	hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	EXITIFFAILED(hr);

	IMFActivate **pDevices = nullptr;
	unsigned int deviceCount = 0;
	hr = MFEnumDeviceSources(pAttributes, &pDevices, &deviceCount);
	EXITIFFAILED(hr);

	for (unsigned int i = 0; i < deviceCount; i++)
	{
		if (SUCCEEDED(pDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pName, &nameLen)) && pName)
		{
			if (!wName.compare(pName))
				pDevice = pDevices[i];
			CoTaskMemFree(pName);
			pName = nullptr;
			if (pDevice)
				break;
			pDevices[i]->Release(); //release unused					
		}
		else
			pDevices[i]->Release(); //release unused				
	}

	if (pDevices)
		CoTaskMemFree(pDevices);

	pAttributes->Release();
	pAttributes = nullptr;

	if (pDevice)
		return new CCaptureDevice(pDevice, pOptions);

exitFail:

	if (pAttributes)
		pAttributes->Release();

	CoUninitialize();
	return nullptr;
}

CCaptureDevice::CCaptureDevice(IMFActivate *pDevice, CaptureOptions *pOptions)
{
	InitializeCriticalSection(&m_CritSec);
	m_Convert = nullptr;
	m_Status = Stopped;
	m_pDevice = pDevice;
	m_pOptions = pOptions;
	m_RefCount = 1;
	m_pSource = nullptr;
	m_pReader = nullptr;
	m_SourceStride = 0;
	m_DestinationStride = 0;
}

CCaptureDevice::~CCaptureDevice()
{
	m_pOptions->Initialized = false;
	Stop();
	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = nullptr;
	}
	CoUninitialize();
	DeleteCriticalSection(&m_CritSec);
}

bool CCaptureDevice::Start()
{
	if (m_Status != Stopped)
		return false;

	HRESULT hr = m_pDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&m_pSource);

	IMFPresentationDescriptor *pPD = NULL;
	IMFStreamDescriptor *pSD = NULL;
	IMFMediaTypeHandler *pHandler = NULL;


	BOOL fSelected;
	DWORD cTypes = 0;

	hr = m_pSource->CreatePresentationDescriptor(&pPD);
	hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
	hr = pSD->GetMediaTypeHandler(&pHandler);
	hr = pHandler->GetMediaTypeCount(&cTypes);

	IMFMediaType *pOriginalMediaType = nullptr;
	UINT32 width = 0;
	UINT32 height = 0;
	UINT32 bestIndex = 0;
	UINT32 frameRateMax = 0;
	UINT32 frameRate = 0;
	UINT32 denominator = 0;
	GUID subtype = { 0 };
	CCaptureDevice::pfConverterFunction pfBestConverter = nullptr;
	CCaptureDevice::pfConverterFunction pfCurrentConverter = nullptr;


	for (DWORD i = 0; i < cTypes; i++)
	{
		hr = pHandler->GetMediaTypeByIndex(i, &pOriginalMediaType);
		if (FAILED(hr))
			continue;
		hr = MFGetAttributeSize(pOriginalMediaType, MF_MT_FRAME_SIZE, &width, &height);
		if (width == m_pOptions->Width && height == m_pOptions->Height)
		{
			hr = MFGetAttributeRatio(pOriginalMediaType, MF_MT_FRAME_RATE_RANGE_MAX, &frameRate, &denominator);
			if (frameRate >= frameRateMax)
			{
				hr = pOriginalMediaType->GetGUID(MF_MT_SUBTYPE, &subtype);		

				//MFT converters are shit, use own
				if (!(pfCurrentConverter = GetConverter(subtype, m_pOptions->MediaFormat)))
					continue;
				if (pfBestConverter && pfCurrentConverter == CCaptureDevice::NullConverter)
					continue; 
				bestIndex = i;
				frameRateMax = frameRate;
				pfBestConverter = pfCurrentConverter;
				
				if (subtype == m_pOptions->MediaFormat)
				{					
					hr = pOriginalMediaType->Release();
					break;
				}
			}
		}		
		hr = pOriginalMediaType->Release();
	}

	m_Convert = pfBestConverter;

	hr = pHandler->GetMediaTypeByIndex(bestIndex, &pOriginalMediaType);
	hr = pHandler->SetCurrentMediaType(pOriginalMediaType);

	hr = pOriginalMediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
	hr = pOriginalMediaType->Release();

	hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, m_pOptions->Width, &m_SourceStride);

	hr = pHandler->Release();
	hr = pSD->Release();
	hr = pPD->Release();	
	
	IMFAttributes *pAttributes = nullptr;
	hr = MFCreateAttributes(&pAttributes, 3);
	hr = pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, FALSE);
	hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE);
	hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
	hr = MFCreateSourceReaderFromMediaSource(m_pSource, pAttributes, &m_pReader);
	hr = pAttributes->Release();

	hr = m_pReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
	hr = m_pReader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, true);

	hr = MFGetStrideForBitmapInfoHeader(m_pOptions->MediaFormat.Data1, m_pOptions->Width, &m_DestinationStride);

	if (!m_Convert)
		return false;
	EnterCriticalSection(&m_CritSec);
	m_Status = Started;
	LeaveCriticalSection(&m_CritSec);
	hr = m_pReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr);	
	return true;
}

bool CCaptureDevice::Stop()
{
	if (m_Status == Stopped)
		return false;	
	EnterCriticalSection(&m_CritSec);
	if (m_pDevice) m_pDevice->ShutdownObject();	
	if (m_pReader) m_pReader->Release(); m_pReader = nullptr;
	if (m_pSource) m_pSource->Release(); m_pSource = nullptr;

	m_Status = Stopped;
	LeaveCriticalSection(&m_CritSec);
	return true;
}

bool CCaptureDevice::Resume()
{
	if (m_Status != Paused)
		return false;
	EnterCriticalSection(&m_CritSec);
	m_Status = Started;
	LeaveCriticalSection(&m_CritSec);
	HRESULT hr = m_pReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr);
	EXITIFFAILED(hr);
	return true;
exitFail:
	EnterCriticalSection(&m_CritSec);
	m_Status = Paused;
	LeaveCriticalSection(&m_CritSec);
	return false;
}

bool CCaptureDevice::Pause()
{
	if (m_Status != Started)
		return false;
	m_Status = Paused;
	return true;
}

// IUnknown methods
STDMETHODIMP CCaptureDevice::QueryInterface(REFIID aRiid, void** aPpv)
{
	static const QITAB qit[] = 
	{
		QITABENT(CCaptureDevice, IMFSourceReaderCallback), 
		{ } 
	};
	return QISearch(this, qit, aRiid, aPpv);
}

STDMETHODIMP_(ULONG) CCaptureDevice::AddRef()
{
	return InterlockedIncrement(&m_RefCount);
}

STDMETHODIMP_(ULONG) CCaptureDevice::Release()
{
	ULONG count = InterlockedDecrement(&m_RefCount);
	if (count == 0)
	{
		delete this;
	}
	// For thread safety, return a temporary variable.
	return count;
}

// IMFSourceReaderCallback methods
STDMETHODIMP CCaptureDevice::OnReadSample(HRESULT aStatus, DWORD aStreamIndex, DWORD aStreamFlags, LONGLONG aTimestamp, IMFSample *aSample)
{
	if (!m_pOptions->Initialized)
		return E_FAIL;

	HRESULT hr = S_OK;

	if (FAILED(aStatus))
		return aStatus;

	EnterCriticalSection(&m_CritSec);
	if (m_Status == Started)
	{
		if (m_pOptions->pCaptureBuffer && SUCCEEDED(aStatus))
		{
			IMFMediaBuffer *pMediaBuffer = nullptr;
			if (aSample)
			{
				hr = aSample->GetBufferByIndex(0, &pMediaBuffer);

				VideoBufferLock buffer(pMediaBuffer);    // Helper object to lock the video buffer.
				BYTE *scanline0 = nullptr;
				LONG stride = 0;
				hr = buffer.LockBuffer(m_SourceStride, m_pOptions->Height, &scanline0, &stride);
				if (stride < 0)
				{
					scanline0 += stride * m_pOptions->Height;
					stride = -stride;
				}
				LONG bytes = stride * m_pOptions->Height;

				m_Convert(scanline0, (BYTE*)m_pOptions->pCaptureBuffer, m_pOptions->Width, m_pOptions->Height, stride, m_DestinationStride);

				if (m_pOptions->pfCallback)
					m_pOptions->pfCallback(this, m_pOptions->pUserData);
			}
			if (pMediaBuffer)
				pMediaBuffer->Release();
		}
		hr = m_pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr);
	}
	LeaveCriticalSection(&m_CritSec);

	return hr;
}

STDMETHODIMP CCaptureDevice::OnEvent(DWORD, IMFMediaEvent *)
{
	return S_OK;
}

STDMETHODIMP CCaptureDevice::OnFlush(DWORD)
{
	return S_OK;
}

CCaptureDevice::pfConverterFunction CCaptureDevice::GetConverter(GUID &from, GUID&to)
{
	if (from == to) return CCaptureDevice::NullConverter;
	CCaptureDevice::pfConverterFunction result = nullptr;
	auto cnt = sizeof(ConverterFunctions) / sizeof(CCaptureDevice::ConverterDef);
	for (unsigned int i=0; i<cnt; i++)
	{
		if (ConverterFunctions[i].SourceFormat == from && ConverterFunctions[i].DestinationFormat == to)
			return ConverterFunctions[i].Convert;
	}
	return result;
}

void CCaptureDevice::NullConverter(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride)
{
	CopyMemory(outputBuffer, inputBuffer, inStride * height);
}

void CCaptureDevice::YUY2toNV12(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride)
{
	/* Color space conversion from YUYV to NV12 */
	//NV12 4:2:0 YYYY UV : 4 pixel = 6 byte
	//YUY2 4:2:2 YUYV : 2 pixel = 4 byte

	uint8_t *Y_H, *Y_L, *U, *V, *I_H, *I_L;
	auto out_ysize = outStride * height;
	auto inPadding = inStride + (inStride - (width << 1));
	auto outPadding = outStride + (outStride - width);

	I_H = inputBuffer;
	I_L = I_H + inStride;

	Y_H = outputBuffer;
	Y_L = Y_H + outStride;

	U = Y_H + out_ysize;
	V = U + 1;

	for (auto h = 0; h < height >> 1; h++)
	{
		for (auto w = 0; w < width >> 1; w++)
		{
			*Y_H = *I_H; I_H++; Y_H++;
			*Y_L = *I_L; I_L++; Y_L++;
			*U = (*I_H + *I_L) / 2; U += 2; I_H++; I_L++;
			*Y_H = *I_H; I_H++; Y_H++;
			*Y_L = *I_L; I_L++; Y_L++;
			*V = (*I_H + *I_L) / 2; V += 2; I_H++; I_L++;
		}
		I_H += inPadding;
		I_L += inPadding;
		Y_H += outPadding;
		Y_L += outPadding;
	}
}

void CCaptureDevice::RGB24toNV12(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride)
{
	//rgb24 is upside down?

	uint8_t *Y_H, *Y_L, *U, *V, *I_H, *I_L;
	uint8_t R, G, B;
	int16_t xU, xV;
	auto out_ysize = outStride * height;
	auto inPadding = (width * 3) + (2 * inStride);
	auto outPadding = outStride + (outStride - width);



	I_H = inputBuffer + (inStride * (height - 2));
	I_L = I_H + inStride;

	Y_H = outputBuffer;
	Y_L = Y_H + outStride;

	U = Y_H + out_ysize;
	V = U + 1;

	for (auto h = 0; h < height >> 1; h++) //we're processing 2 line at once
	{
		for (auto w = 0; w < width >> 1; w++) //we process 6 bytes
		{
			B = *I_H; I_H++;
			G = *I_H; I_H++;
			R = *I_H; I_H++;

			*Y_H = (uint8_t)((R * 66 + G * 129 + B * 25 + 128) >> 8) + 16; Y_H++;

			xU = ((R * -38 - G * 74 + B * 112 + 128) >> 8) + 128;
			xV = ((R * 112 - G * 94 - B * 18 + 128) >> 8) + 128;

			B = *I_H; I_H++;
			G = *I_H; I_H++;
			R = *I_H; I_H++;

			*Y_H = (uint8_t)((R * 66 + G * 129 + B * 25 + 128) >> 8) + 16; Y_H++;

			xU += ((R * -38 - G * 74 + B * 112 + 128) >> 8) + 128;
			xV += ((R * 112 - G * 94 - B * 18 + 128) >> 8) + 128;

			B = *I_L; I_L++;
			G = *I_L; I_L++;
			R = *I_L; I_L++;

			*Y_L = (uint8_t)((R * 66 + G * 129 + B * 25 + 128) >> 8) + 16; Y_L++;

			xU += ((R * -38 - G * 74 + B * 112 + 128) >> 8) + 128;
			xV += ((R * 112 - G * 94 - B * 18 + 128) >> 8) + 128;

			B = *I_L; I_L++;
			G = *I_L; I_L++;
			R = *I_L; I_L++;

			*Y_L = (uint8_t)((R * 66 + G * 129 + B * 25 + 128) >> 8) + 16; Y_L++;

			xU += ((R * -38 - G * 74 + B * 112 + 128) >> 8) + 128;
			xV += ((R * 112 - G * 94 - B * 18 + 128) >> 8) + 128;

			*U = (uint8_t)(xU / 4); U += 2;
			*V = (uint8_t)(xV / 4); V += 2;
		}
		I_H -= inPadding;
		I_L -= inPadding;
		Y_H += outPadding;
		Y_L += outPadding;
	}	
}


/*
#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// RGB -> YUV
#define RGB2Y(R, G, B) CLIP(( (  66 * (R) + 129 * (G) +  25 * (B) + 128) >> 8) +  16)
#define RGB2U(R, G, B) CLIP(( ( -38 * (R) -  74 * (G) + 112 * (B) + 128) >> 8) + 128)
#define RGB2V(R, G, B) CLIP(( ( 112 * (R) -  94 * (G) -  18 * (B) + 128) >> 8) + 128)

// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)

// RGB -> YCbCr
#define CRGB2Y(R, G, B) CLIP((19595 * R + 38470 * G + 7471 * B ) >> 16)
#define CRGB2Cb(R, G, B) CLIP((36962 * (B - CLIP((19595 * R + 38470 * G + 7471 * B ) >> 16) ) >> 16) + 128)
#define CRGB2Cr(R, G, B) CLIP((46727 * (R - CLIP((19595 * R + 38470 * G + 7471 * B ) >> 16) ) >> 16) + 128)

// YCbCr -> RGB
#define CYCbCr2R(Y, Cb, Cr) CLIP( Y + ( 91881 * Cr >> 16 ) - 179 )
#define CYCbCr2G(Y, Cb, Cr) CLIP( Y - (( 22544 * Cb + 46793 * Cr ) >> 16) + 135)
#define CYCbCr2B(Y, Cb, Cr) CLIP( Y + (116129 * Cb >> 16 ) - 226 )



void Bitmap2Yuv420p( boost::uint8_t *destination, boost::uint8_t *rgb,
const int &width, const int &height ) {
const size_t image_size = width * height;
boost::uint8_t *dst_y = destination;
boost::uint8_t *dst_u = destination + image_size;
boost::uint8_t *dst_v = destination + image_size + image_size/4;

// Y plane
for( size_t i = 0; i < image_size; ++i ) {
*dst_y++ = ( ( 66*rgb[3*i] + 129*rgb[3*i+1] + 25*rgb[3*i+2] ) >> 8 ) + 16;
}
#if 1
// U plane
for( size_t y=0; y<height; y+=2 ) {
for( size_t x=0; x<width; x+=2 ) {
const size_t i = y*width + x;
*dst_u++ = ( ( -38*rgb[3*i] + -74*rgb[3*i+1] + 112*rgb[3*i+2] ) >> 8 ) + 128;
}
// V plane
for( size_t y=0; y<height; y+=2 ) {
for( size_t x=0; x<width; x+=2 ) {
const size_t i = y*width + x;
*dst_v++ = ( ( 112*rgb[3*i] + -94*rgb[3*i+1] + -18*rgb[3*i+2] ) >> 8 ) + 128;
}
#else // also try this version:
// U+V planes
for( size_t y=0; y<height; y+=2 ) {
for( size_t x=0; x<width; x+=2 ) {
const size_t i = y*width + x;
*dst_u++ = ( ( -38*rgb[3*i] + -74*rgb[3*i+1] + 112*rgb[3*i+2] ) >> 8 ) + 128;
*dst_v++ = ( ( 112*rgb[3*i] + -94*rgb[3*i+1] + -18*rgb[3*i+2] ) >> 8 ) + 128;
}
#endif
}

*/

// Clamp out of range values
#define CLAMP(t) (((t)>255)?255:(((t)<0)?0:(t)))
// Color space conversion for RGB
#define GET_R_FROM_YUV(y, u, v) ((298*y+409*v+128)>>8)
#define GET_G_FROM_YUV(y, u, v) ((298*y-100*u-208*v+128)>>8)
#define GET_B_FROM_YUV(y, u, v) ((298*y+516*u+128)>>8)

void CCaptureDevice::YUY2toRGB24(uint8_t *inputBuffer, uint8_t *outputBuffer, int width, int height, int inStride, int outStride)
{

	for (int i = 0; i < height*(width / 2); i++)
	{
		int y1 = *inputBuffer+ - 16;
		int u = *inputBuffer++ - 128;
		int y2 = *inputBuffer++ - 16;
		int v = *inputBuffer++ - 128;

		// BGR
		*outputBuffer++ = CLAMP(GET_B_FROM_YUV(y1, u, v));
		*outputBuffer++ = CLAMP(GET_G_FROM_YUV(y1, u, v));
		*outputBuffer++ = CLAMP(GET_R_FROM_YUV(y1, u, v));

		// BGR
		*outputBuffer++ = CLAMP(GET_B_FROM_YUV(y2, u, v));
		*outputBuffer++ = CLAMP(GET_G_FROM_YUV(y2, u, v));
		*outputBuffer++ = CLAMP(GET_R_FROM_YUV(y2, u, v));

		
	}
}

