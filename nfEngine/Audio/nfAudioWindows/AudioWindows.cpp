/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   DLL entry point definition for nfAudioWindows project
 */

#include "PCH.hpp"
#include "AudioWindows.hpp"



BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    UNUSED(hModule);
    UNUSED(lpReserved);

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
