#include "Common.h"

//rev-eng attempt to detect if monitor is connected and in direct mode

class ALVRDisplayEnumerator
{
public:
	virtual void ALVR_STD_CALL DummyFunc0() = 0;
	virtual void ALVR_STD_CALL DummyFunc1() = 0;
	virtual void ALVR_STD_CALL DummyFunc2() = 0;
	virtual void ALVR_STD_CALL DummyFunc3() = 0;
	virtual ALVR_RESULT ALVR_STD_CALL GetDisplay(void **ppDisplay) = 0;
};

class ALVRDisplay
{
public:
	virtual void ALVR_STD_CALL DummyFunc0() = 0;
	virtual void ALVR_STD_CALL DummyFunc1() = 0;
	virtual void ALVR_STD_CALL DummyFunc2() = 0;
	virtual void ALVR_STD_CALL DummyFunc3() = 0;
	virtual void ALVR_STD_CALL DummyFunc4() = 0;
	virtual void ALVR_STD_CALL DummyFunc5() = 0;
	virtual void ALVR_STD_CALL DummyFunc6() = 0;
	virtual void ALVR_STD_CALL DummyFunc7() = 0;
	virtual void ALVR_STD_CALL DummyFunc8() = 0;
	virtual void ALVR_STD_CALL DummyFunc9() = 0;
	virtual void ALVR_STD_CALL DummyFunc10() = 0;
	virtual uint16_t ALVR_STD_CALL GetEDID() = 0;
};


class ALVRDisplayManager
{
public:
	virtual void ALVR_STD_CALL DummyFunc0() = 0;
	virtual void ALVR_STD_CALL DummyFunc1() = 0;
	virtual void ALVR_STD_CALL DummyFunc2() = 0;
	virtual void ALVR_STD_CALL DummyFunc3() = 0;
	virtual void ALVR_STD_CALL DummyFunc4() = 0;
	virtual void ALVR_STD_CALL DummyFunc5() = 0;
	virtual void ALVR_STD_CALL DummyFunc6() = 0;
	virtual void ALVR_STD_CALL DummyFunc7() = 0;
	virtual void ALVR_STD_CALL DummyFunc8() = 0;
	virtual ALVR_RESULT ALVR_STD_CALL EnumerateDisplays(int mode, void **ppDisplayEnum) = 0;
};

class ALVRDisplayFactory
{
public:
	virtual ALVR_RESULT ALVR_STD_CALL CreateManager(int wth, void **ppManager) = 0;
};

bool IsD2DConnected(uint16_t edid)
{
#ifdef _WIN64
	HMODULE hModule = LoadLibrary(L"amdlvr64.dll");
#elif defined WIN32
	HMODULE hModule = LoadLibrary(L"amdlvr32.dll");
#endif
	ALVR_RESULT result = ALVR_FALSE;
	uint64_t version = ALVR_FULL_VERSION;

	if (hModule)
	{
		ALVRDisplayFactory *ppDisplayInitFactory = nullptr;
		ALVRInit_Fn pfALVRDisplayInit = (ALVRInit_Fn)GetProcAddress(hModule, "ALVRDisplayInit");
		if (pfALVRDisplayInit && pfALVRDisplayInit(version, (void **)&ppDisplayInitFactory) == ALVR_OK)
		{
			ALVRFactory *pInitFactory = nullptr;
			ALVRInit_Fn pfALVRInit = (ALVRInit_Fn)GetProcAddress(hModule, "ALVRInit");
			if (pfALVRInit && pfALVRInit(version, (void **)&pInitFactory) == ALVR_OK)
			{
				ALVRDisplayManager *pDisplayManager = nullptr;
				if (ppDisplayInitFactory->CreateManager(0, (void **)&pDisplayManager) == ALVR_OK)
				{
					ALVRDisplayEnumerator *pDisplayEnum = nullptr;
					if (pDisplayManager->EnumerateDisplays(2, (void **)&pDisplayEnum) == ALVR_OK) //0 = inactive, 2 = active D2D displays?
					{
						ALVRDisplay *pDisplay = nullptr;
						auto result = pDisplayEnum->GetDisplay((void**)&pDisplay);
						while (result == ALVR_OK)
						{
							uint16_t EDID = pDisplay->GetEDID();
							if (EDID == edid)
								return true;
							pDisplay = nullptr;
							result = pDisplayEnum->GetDisplay((void **)&pDisplay);
						}
					}
				}
			}
		}

		FreeLibrary(hModule);
		hModule = nullptr;
	}
	return false;
}
