/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  DLL entry point.
 */

#include "PCH.hpp"
#include "Core.hpp"
#include "nfCommon/Logger/Logger.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}
