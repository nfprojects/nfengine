/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   DLL entry point definition for Vulkan renderer
 */

#include "PCH.hpp"

#include "Defines.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    UNUSED(hModule);
    UNUSED(ul_reason_for_call);
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
