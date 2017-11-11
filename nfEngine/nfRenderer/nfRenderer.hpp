/**
* @file
* @author Witek902 (witek902@gmail.com)
* @brief  Basic types declarations. This file will be included by most of the engine's headers.
*/

#pragma once

#include "nfCommon/nfCommon.hpp"


// DLL import / export macro
#ifdef NF_RENDERER_EXPORTS
#define NFE_RENDERER_API __declspec(dllexport)
#else
#define NFE_RENDERER_API __declspec(dllimport)
#endif