/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Precompiled header for nfAudioWindows project
 */

#pragma once

 // enable memory allocation tracking
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(_DEBUG)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <xaudio2.h>

#include <new>
#include <memory>
#include <assert.h>