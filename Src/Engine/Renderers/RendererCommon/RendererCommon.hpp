/**
 * @file
 * @author  Witek902
 * @brief   Main header for RendererCommon project
 */

#pragma once

#include "../../Common/nfCommon.hpp"

 // DLL import / export macro
#ifdef WIN32
#ifdef NFE_RENDERER_COMMON_EXPORTS
#define NFE_RENDERER_COMMON_API NFE_API_EXPORT
#else // NFE_RENDERER_COMMON_EXPORTS
#define NFE_RENDERER_COMMON_API NFE_API_IMPORT
#endif // NFE_RENDERER_COMMON_EXPORTS
#else // WIN32
#define NFE_RENDERER_COMMON_API __attribute__((visibility("default")))
#endif // WIN32
