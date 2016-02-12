#pragma once

#ifndef Common_H
#define Common_H

#include <Windows.h>
#include <openvr_driver.h>
#include <vector>
#include <memory>
#include <string>

#define TRACE(a) OutputDebugString(a"\n")

#define HMD_DLL_EXPORT extern "C" __declspec(dllexport)
#define HMD_DLL_IMPORT extern "C" __declspec(dllimport)

#endif // Common_H