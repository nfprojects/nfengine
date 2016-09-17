/**
* @file
* @author  Witek902 (witek902@gmail.com)
* @brief   Common utilities definitions for D3D12 renderer.
*/

#pragma once

#include "PCH.hpp"
#include "Common.hpp"
#include "nfCommon/Logger/Logger.hpp"


namespace NFE {
namespace Renderer {

namespace {

const char* TranslateHResult(HRESULT hr)
{
    switch (hr)
    {
    case D3D12_ERROR_ADAPTER_NOT_FOUND:
        return "D3D12_ERROR_ADAPTER_NOT_FOUND";
    case D3D12_ERROR_DRIVER_VERSION_MISMATCH:
        return "D3D12_ERROR_DRIVER_VERSION_MISMATCH";
    case DXGI_ERROR_INVALID_CALL:
        return "DXGI_ERROR_INVALID_CALL";
    case DXGI_ERROR_WAS_STILL_DRAWING:
        return "DXGI_ERROR_WAS_STILL_DRAWING";
    case E_FAIL:
        return "E_FAIL";
    case E_INVALIDARG:
        return "E_INVALIDARG";
    case E_OUTOFMEMORY:
        return "E_OUTOFMEMORY";
    case E_NOTIMPL:
        return "E_NOTIMPL";
    case S_FALSE:
        return "S_FALSE";
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

} // namespace Renderer
} // namespace NFE
