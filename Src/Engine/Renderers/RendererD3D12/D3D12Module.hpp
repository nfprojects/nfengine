/**
 * @file
 * @author  Witek902
 * @brief   Declaration of Direct3D 12 SDK version.
 */

#pragma once

#include "../../Common/nfCommon.hpp"

#if defined(NFE_PLATFORM_WINDOWS)

extern "C"
{
    __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 618;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\";
}

#endif  // defined(NFE_PLATFORM_WINDOWS)
