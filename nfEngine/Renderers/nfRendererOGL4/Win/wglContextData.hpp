/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Windows-specific structure wglContextData, managed by Context instance
 */

#pragma once

#if !defined(WIN32)
#error "wglContextData header is Windows specific and shouldn't be a part of other platform's code."
#endif

struct wglContextData
{

};
