/* Extremely Simple Capture API */

typedef void (*pfCameraFrameCallback)(char *pFrame, int width, int height, int stride, GUID *pMediaFormat, void *pUserData);

struct SimpleCapParams
{
	/* Target buffer. 
	 * Must be at least mWidth * mHeight * sizeof(int) of size! 
	 */
	int * mTargetBuf;
	/* Buffer width */
	int mWidth;
	/* Buffer height */
	int mHeight;
	GUID *pMediaFormat;
	LONG *pStride;
	void *pUserData;
	int Options;
	pfCameraFrameCallback pfCallback;
};

enum CAPTURE_PROPETIES
{
	CAPTURE_BRIGHTNESS,
	CAPTURE_CONTRAST,
	CAPTURE_HUE,
	CAPTURE_SATURATION,
	CAPTURE_SHARPNESS,
	CAPTURE_GAMMA,
	CAPTURE_COLORENABLE,
	CAPTURE_WHITEBALANCE,
	CAPTURE_BACKLIGHTCOMPENSATION,
	CAPTURE_GAIN,
	CAPTURE_PAN,
	CAPTURE_TILT,
	CAPTURE_ROLL,
	CAPTURE_ZOOM,
	CAPTURE_EXPOSURE,
	CAPTURE_IRIS,
	CAPTURE_FOCUS,
	CAPTURE_PROP_MAX
};



// Options accepted by above:
// Return raw data instead of converted rgb. Using this option assumes you know what you're doing.
#define CAPTURE_OPTION_RAWDATA 1 
// Mask to check for valid options - all options OR:ed together.
#define CAPTURE_OPTIONS_MASK (CAPTURE_OPTION_RAWDATA) 

void getCaptureDeviceName(unsigned int deviceno, char *namebuffer, int bufferlength);
//int ESCAPIDLLVersion();
//int ESCAPIVersion();
int countCaptureDevices();
void initCOM();
int initCapture(unsigned int deviceno, struct SimpleCapParams *aParams);
void deinitCapture(unsigned int deviceno);
//void doCapture(unsigned int deviceno);
//int isCaptureDone(unsigned int deviceno);
int getCaptureErrorLine(unsigned int deviceno);
int getCaptureErrorCode(unsigned int deviceno);
float getCapturePropertyValue(unsigned int deviceno, int prop);
int getCapturePropertyAuto(unsigned int deviceno, int prop);
int setCaptureProperty(unsigned int deviceno, int prop, float value, int autoval);
//int initCaptureWithOptions(unsigned int deviceno, struct SimpleCapParams *aParams, unsigned int aOptions);
