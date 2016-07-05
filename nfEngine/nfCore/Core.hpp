/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic types declarations. This file will be included by most of the engine's headers.
 */

#pragma once

#include "Prerequisites.hpp"
#include "nfCommon/nfCommon.hpp"


 // DLL import / export macro
#ifdef NF_CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif