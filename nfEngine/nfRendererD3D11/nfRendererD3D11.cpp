#include "stdafx.hpp"
#include "nfRendererD3D11.hpp"
#include "Renderer.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Render {

// entry point for the DLL
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}


IRenderer* GetInstance()
{
    if (RendererD3D11::GetInstance() != NULL)
        return RendererD3D11::GetInstance(); // renderer is already created

    RendererD3D11* pRenderer = new RendererD3D11;
    RendererD3D11::GetInstance() = pRenderer;
    return pRenderer;
}


const char* XTranslateHResult(HRESULT hr)
{
    switch (hr)
    {
        case D3D11_ERROR_FILE_NOT_FOUND:
            return "D3D11_ERROR_FILE_NOT_FOUND";

        case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
            return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";

        case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
            return "D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS";

        case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
            return "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";

        case E_FAIL:
            return "E_FAIL";

        case E_INVALIDARG:
            return "E_INVALIDARG";

        case E_OUTOFMEMORY:
            return "E_OUTOFMEMORY";

        case S_FALSE:
            return "S_FALSE";
    }

    return "Unknown error";
}

/*
    DirectX API error handling
    Decode HRESULT, display error message box with detailed information.
*/
HRESULT D3DError(HRESULT hr, const char* pStr, const char* pFile, int line, bool critical)
{
    if (FAILED(hr))
    {
        std::string messageStr;
        char tmpStr[16];

        LOG_ERROR("Direct3D call \"%s\" failed. HR = 0x%08X (%s)", pStr, hr, XTranslateHResult(hr));

        if (critical)
        {
            messageStr = "An error occurred during Direct3D API call:\n";

            messageStr += "\"";
            messageStr += pStr;
            messageStr += "\"\n";

            messageStr += "at line ";
            messageStr += std::to_string(line);
            messageStr += " of source file \"";
            messageStr += pFile;
            messageStr += "\".\n\n";

            sprintf_s(tmpStr, "0x%08X", static_cast<uint32>(hr));
            messageStr += "HRESULT = ";
            messageStr += tmpStr;
            messageStr += " (";
            messageStr += XTranslateHResult(hr);
            messageStr +=
                ")\n\nPress OK to terminate the application.\nPress Cancel to continue. It may result in undefined behavior.";

            int result = MessageBoxA(0, messageStr.c_str(), "NFEngine - Direct3D Error",
                                     MB_ICONERROR | MB_OKCANCEL);
            if (result == IDOK) ExitProcess(1);
        }
    }

    return hr;
}

} // namespace Render
} // namespace NFE
