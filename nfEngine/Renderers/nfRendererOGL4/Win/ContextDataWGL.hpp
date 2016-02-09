/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Windows-specific structure ContextDataWGL, managed by Context instance
 */

#pragma once

#if !defined(WIN32)
#error "ContextDataWGL header is Windows specific and shouldn't be a part of other platform's code."
#endif

struct ContextDataWGL
{

};
