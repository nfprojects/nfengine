/**
* @file
* @author  Witek902 (witek902@gmail.com)
* @brief   Common utilities definitions for D3D12 renderer.
*/

#pragma once

#include "PCH.hpp"
#include "Common.hpp"
#include "RendererD3D12.hpp"
#include "Engine/Common/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

namespace {

const char* TranslateHResult(HRESULT hr)
{
    switch (hr)
    {
    case D3D12_ERROR_ADAPTER_NOT_FOUND:             return "D3D12_ERROR_ADAPTER_NOT_FOUND";
    case D3D12_ERROR_DRIVER_VERSION_MISMATCH:       return "D3D12_ERROR_DRIVER_VERSION_MISMATCH";

    case DXGI_ERROR_INVALID_CALL:                   return "DXGI_ERROR_INVALID_CALL";
    case DXGI_ERROR_NOT_FOUND:                      return "DXGI_ERROR_NOT_FOUND";
    case DXGI_ERROR_MORE_DATA:                      return "DXGI_ERROR_MORE_DATA";
    case DXGI_ERROR_UNSUPPORTED:                    return "DXGI_ERROR_UNSUPPORTED";
    case DXGI_ERROR_DEVICE_REMOVED:                 return "DXGI_ERROR_DEVICE_REMOVED";
    case DXGI_ERROR_DEVICE_HUNG:                    return "DXGI_ERROR_DEVICE_HUNG";
    case DXGI_ERROR_DEVICE_RESET:                   return "DXGI_ERROR_DEVICE_RESET";
    case DXGI_ERROR_WAS_STILL_DRAWING:              return "DXGI_ERROR_WAS_STILL_DRAWING";
    case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:      return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
    case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:   return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
    case DXGI_ERROR_DRIVER_INTERNAL_ERROR:          return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
    case DXGI_ERROR_NONEXCLUSIVE:                   return "DXGI_ERROR_NONEXCLUSIVE";
    case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:        return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
    case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:     return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";
    case DXGI_ERROR_REMOTE_OUTOFMEMORY:             return "DXGI_ERROR_REMOTE_OUTOFMEMORY";
    case DXGI_ERROR_ACCESS_LOST:                    return "DXGI_ERROR_ACCESS_LOST";
    case DXGI_ERROR_WAIT_TIMEOUT:                   return "DXGI_ERROR_WAIT_TIMEOUT";
    case DXGI_ERROR_SESSION_DISCONNECTED:           return "DXGI_ERROR_SESSION_DISCONNECTED";
    case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:       return "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE";
    case DXGI_ERROR_CANNOT_PROTECT_CONTENT:         return "DXGI_ERROR_CANNOT_PROTECT_CONTENT";
    case DXGI_ERROR_ACCESS_DENIED:                  return "DXGI_ERROR_ACCESS_DENIED";
    case DXGI_ERROR_NAME_ALREADY_EXISTS:            return "DXGI_ERROR_NAME_ALREADY_EXISTS";
    case DXGI_ERROR_SDK_COMPONENT_MISSING:          return "DXGI_ERROR_SDK_COMPONENT_MISSING";
    case DXGI_ERROR_NOT_CURRENT:                    return "DXGI_ERROR_NOT_CURRENT";
    case DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY:      return "DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY";
    case DXGI_ERROR_DYNAMIC_CODE_POLICY_VIOLATION:  return "DXGI_ERROR_DYNAMIC_CODE_POLICY_VIOLATION";
    case DXGI_ERROR_NON_COMPOSITED_UI:              return "DXGI_ERROR_NON_COMPOSITED_UI";

    case E_ACCESSDENIED:                            return "E_ACCESSDENIED";
    case E_ABORT:                                   return "E_ABORT";
    case E_FAIL:                                    return "E_FAIL";
    case E_HANDLE:                                  return "E_HANDLE";
    case E_INVALIDARG:                              return "E_INVALIDARG";
    case E_OUTOFMEMORY:                             return "E_OUTOFMEMORY";
    case E_NOTIMPL:                                 return "E_NOTIMPL";
    case E_NOINTERFACE:                             return "E_NOINTERFACE";
    case E_POINTER:                                 return "E_POINTER";
    case E_UNEXPECTED:                              return "E_UNEXPECTED";
    case S_FALSE:                                   return "S_FALSE";
    }

    return "Unknown error";
}

} // namespace

HRESULT D3DError(HRESULT hr, const char* srcFile, int line)
{
    if (FAILED(hr))
    {
        NFE_LOG_ERROR("Direct3D call in %s:%d failed with code 0x%08X (%s)", srcFile, line, hr, TranslateHResult(hr));
        ::DebugBreak();
    }

    return hr;
}

bool SetDebugName(ID3D12Object* obj, const Common::StringView name)
{
#ifndef NFE_CONFIGURATION_FINAL
    if (gDevice->IsDebugLayerEnabled() && !name.Empty())
    {
        Common::Utf16String longName;
        if (Common::UTF8ToUTF16(name, longName))
        {
            if (FAILED(D3D_CALL_CHECK(obj->SetName(longName.c_str()))))
            {
                NFE_LOG_ERROR("Failed to set debug name for D3D object: '%.*s'", name.Length(), name.Data());
                return false;
            }
        }
        else
        {
            return false;
        }
    }
#else
    NFE_UNUSED(obj);
    NFE_UNUSED(name);
#endif // NFE_CONFIGURATION_FINAL

    return true;
}

} // namespace Renderer
} // namespace NFE
