
#include <Windows.h>
#define OPENVR_API_NODLL 
#include <openvr_capi.h>
#undef OPENVR_API_NODLL 
#include <string>
#include "OverlayManager.h"


#define LOAD(a) m_##a = 0; m_##a = (intptr_t) GetProcAddress(m_hLibHandle, #a)
#define GETINTERFACE(i,e) m_Interfaces.m_p##i = ((pfVR_GetGenericInterface)m_VR_GetGenericInterface)(I##i##_Version, &e)				
#define _V(i) ((VR_IVR##i##_FnTable*)m_Interfaces.m_pVR##i)
#define _F(f) ((pf##f)m_##f)

namespace vr 
{

	typedef intptr_t( *pfVR_InitInternal)(EVRInitError *peError, EVRApplicationType eApplicationType);
	typedef void ( *pfVR_ShutdownInternal)();
	typedef bool ( *pfVR_IsInterfaceVersionValid)(const char *pchInterfaceVersion);
	typedef intptr_t( *pfVR_GetInitToken)();
	typedef intptr_t( *pfVR_GetGenericInterface)(const char *pchInterfaceVersion, EVRInitError *peError);
	typedef const char *( *pfVR_GetVRInitErrorAsEnglishDescription)(EVRInitError error);
	typedef const char *( *pfVR_GetVRInitErrorAsSymbol)(EVRInitError error);
	typedef const char *( *pfVR_RuntimePath)();
	typedef bool ( *pfVR_IsRuntimeInstalled)();
	typedef bool ( *pfVR_IsHmdPresent)();

	COverlayManager::COverlayManager()
	{
		m_Interfaces = { 0 };

		m_hLibHandle = LoadLibrary(L"openvr_api.dll");
		if (!m_hLibHandle)
			return;

		LOAD(VR_InitInternal);
		LOAD(VR_ShutdownInternal);
		LOAD(VR_IsHmdPresent);
		LOAD(VR_GetGenericInterface);
		LOAD(VR_IsRuntimeInstalled);
		LOAD(VR_IsRuntimeInstalled);
		LOAD(VR_IsInterfaceVersionValid);
		LOAD(VR_GetInitToken);
		LOAD(VR_GetVRInitErrorAsEnglishDescription);
		LOAD(VR_GetVRInitErrorAsSymbol);
		LOAD(VR_RuntimePath);
		 
		m_OverlayHandle = 0;
		EVRInitError eVRInitError;
		VR_Init(&eVRInitError, EVRApplicationType_VRApplication_VRMonitor);
		if (!m_Interfaces.m_pVRSystem || eVRInitError != EVRInitError_VRInitError_None)
			return;

		GETINTERFACE(VRChaperone, eVRInitError);
		GETINTERFACE(VRChaperoneSetup, eVRInitError);
		GETINTERFACE(VRCompositor, eVRInitError);
		GETINTERFACE(VROverlay, eVRInitError);
		GETINTERFACE(VRRenderModels, eVRInitError);
		GETINTERFACE(VRExtendedDisplay, eVRInitError);
		GETINTERFACE(VRSettings, eVRInitError);
		GETINTERFACE(VRApplications, eVRInitError);
		GETINTERFACE(VRTrackedCamera, eVRInitError);
		 
	}
	
	COverlayManager::~COverlayManager()
	{
		if (!m_hLibHandle)
			return;

		HideOverlay();
		VR_Shutdown();

		FreeLibrary(m_hLibHandle);
		m_hLibHandle = nullptr;		
	}

	void COverlayManager::CheckClear()
	{
		if (m_VRToken != _F(VR_GetInitToken)())
		{
			m_Interfaces = { 0 };
			m_VRToken = _F(VR_GetInitToken)();
		}
	}

	//void COverlayManager::HandleEvents()
	//{
	//	vr::VREvent_t Event;
	//	if (vr::VRSystem()->PollNextEvent(&Event, sizeof(Event)))
	//	{
	//		switch (Event.eventType)
	//		{
	//		case vr::VREvent_VendorSpecific_Reserved_Start + 0:
	//			ShowOverlay();
	//			break;
	//		}
	//	}
	//}

	void COverlayManager::ShowOverlay()
	{
		HideOverlay();
		// Compositor must be initialized to create overlays
		if (!m_Interfaces.m_pVRCompositor)
			return;

		EVROverlayError eOverlayError = _V(Overlay)->CreateOverlay("customhmd", "Test Overlay", &m_OverlayHandle);
		if (eOverlayError != EVROverlayError_VROverlayError_None)
			return;

		HmdMatrix34_t matInFrontOfHead;
		memset(&matInFrontOfHead, 0, sizeof(matInFrontOfHead));
		float scale = 1.4f;
		matInFrontOfHead.m[0][0] = matInFrontOfHead.m[1][1] = matInFrontOfHead.m[2][2] = scale;
		matInFrontOfHead.m[2][3] = -2.0f;
		eOverlayError = _V(Overlay)->SetOverlayTransformTrackedDeviceRelative(m_OverlayHandle, k_unTrackedDeviceIndex_Hmd, &matInFrontOfHead);
		if (eOverlayError != EVROverlayError_VROverlayError_None)
			return;

		std::string image = "D:\\Media\\Photos\\Anne.jpg";

		eOverlayError = _V(Overlay)->SetOverlayFromFile(m_OverlayHandle, (char *)image.c_str());
		if (eOverlayError != EVROverlayError_VROverlayError_None)
			return;

		eOverlayError = _V(Overlay)->ShowOverlay(m_OverlayHandle);
		if (eOverlayError != EVROverlayError_VROverlayError_None)
			return;

	}

	void COverlayManager::HideOverlay()
	{
		if (m_OverlayHandle == 0)
			return;		
		_V(Overlay)->HideOverlay(m_OverlayHandle);
		_V(Overlay)->DestroyOverlay(m_OverlayHandle);
		m_OverlayHandle = 0;
	}

	void COverlayManager::VR_Init(EVRInitError *peError, EVRApplicationType eApplicationType)
	{
		
		EVRInitError eError;
		m_VRToken = _F(VR_InitInternal)(&eError, eApplicationType);

		if (eError == EVRInitError_VRInitError_None)
		{
			if (_F(VR_IsInterfaceVersionValid)(IVRSystem_Version))
			{
				GETINTERFACE(VRSystem, eError);
			}
			else
			{
				_F(VR_ShutdownInternal)();
				eError = EVRInitError_VRInitError_Init_InterfaceNotFound;
			}
		}

		if (peError)
			*peError = eError;		
	}

	void COverlayManager::VR_Shutdown()
	{
		_F(VR_ShutdownInternal)();
	}

};