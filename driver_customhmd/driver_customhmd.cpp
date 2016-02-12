// Internal Includes
#include "Common.h"

#include "ServerDriver.h" // for ServerDriver_OSVR
#include "ClientDriver.h" // for ClientDriver_OSVR

// Standard includes
#include <cstring> // for std::strcmp

static CServerDriver g_ServerDriver;
static CClientDriver g_ClientDriver;


HMD_DLL_EXPORT void* HmdDriverFactory(const char* interface_name, int* return_code)
{
	if (return_code) {
		*return_code = vr::VRInitError_None;
	}

	if (0 == std::strcmp(vr::IServerTrackedDeviceProvider_Version, interface_name)) {
		return &g_ServerDriver;
	}

	if (0 == std::strcmp(vr::IClientTrackedDeviceProvider_Version, interface_name)) {
		return &g_ClientDriver;
	}

	if (return_code) {
		*return_code = vr::VRInitError_Init_InterfaceNotFound;
	}

	return NULL;
}