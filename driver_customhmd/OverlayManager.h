#pragma once

//include nothing
namespace vr 
{
	class COverlayManager
	{
	public:
		COverlayManager();
		~COverlayManager();
		//void HandleEvents();	
		void ShowOverlay();
		void HideOverlay();
	
	private:
		VROverlayHandle_t m_OverlayHandle;
		HMODULE m_hLibHandle;
		intptr_t m_VR_InitInternal;
		intptr_t m_VR_ShutdownInternal;
		intptr_t m_VR_IsInterfaceVersionValid;
		intptr_t m_VR_GetInitToken;
		intptr_t m_VR_GetGenericInterface;
		intptr_t m_VR_GetVRInitErrorAsEnglishDescription;
		intptr_t m_VR_GetVRInitErrorAsSymbol;
		intptr_t m_VR_RuntimePath;
		intptr_t m_VR_IsRuntimeInstalled;
		intptr_t m_VR_IsHmdPresent;

	private:
		uint32_t m_VRToken;
		struct VRInterfaces
		{			
			intptr_t m_pVRSystem;
			intptr_t m_pVRChaperone;
			intptr_t m_pVRChaperoneSetup;
			intptr_t m_pVRCompositor;
			intptr_t m_pVROverlay;
			intptr_t m_pVRRenderModels;
			intptr_t m_pVRExtendedDisplay;
			intptr_t m_pVRSettings;
			intptr_t m_pVRApplications;
			intptr_t m_pVRTrackedCamera;
		};

		VRInterfaces m_Interfaces;
		void CheckClear();
		void VR_Shutdown();
		void VR_Init(EVRInitError *peError, EVRApplicationType eApplicationType);
	};
}