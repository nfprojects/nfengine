/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Common utilities for D3D12 renderer.
 */

#pragma once

#include "Engine/Common/nfCommon.hpp"
#include "Engine/Common/System/Win/Common.hpp"
#include "D3DPtr.hpp"


namespace NFE {
namespace Renderer {

/**
 * DirectX API error handling
 * @param hr           Result of a D3D call
 * @param srcFile,line Source file name and line with the call
 */
HRESULT D3DError(HRESULT hr, const char* srcFile, int line);

#ifndef D3D_CALL_CHECK
#define D3D_CALL_CHECK(x) D3DError((x), __FILE__, __LINE__)
#endif

template<typename T>
bool SetDebugName(T* obj, const Common::StringView name)
{
    if (gDevice->IsDebugLayerEnabled() && !name.Empty())
    {
        Common::Utf16String longName;
        if (Common::UTF8ToUTF16(name, longName))
            return SUCCEEDED(D3D_CALL_CHECK(obj->SetName(longName.c_str())));
        else
            return false;
    }

    return true;
}

} // namespace Renderer
} // namespace NFE
