/**
 * @file   Main.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Entry point of DLL
 */

#include "stdafx.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;

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
