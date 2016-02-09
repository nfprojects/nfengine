/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Linux-specific structure glXContextData, managed by Context instance
 */

#pragma once

#if !defined(__linux__) && !defined(__LINUX__)
#error "glXContextData header is Linux-specific and shouldn't be a part of other platform's code."
#endif

#include <GL/glx.h>
#include <X11/Xlib.h>

struct glXContextData
{
    Display* mDisplay; // TODO replace with std::shared_ptr
    GLXFBConfig mBestFB;
    GLXContext mContext;
};
