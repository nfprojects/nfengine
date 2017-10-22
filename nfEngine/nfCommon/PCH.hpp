/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Precompiled header.
 */

#pragma once

// enable memory allocation tracking (Windows only)
#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(WIN32) && defined(_DEBUG)

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <VersionHelpers.h>
#endif // defined(WIN32)


#ifdef NFE_USE_SSE
#include <xmmintrin.h>
#endif // NFE_USE_SSE

#ifdef NFE_USE_SSE4
#include <smmintrin.h>
#endif // NFE_USE_SSE4

#if defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)
#include <immintrin.h>
#endif // defined(NFE_USE_AVX2) | defined(NFE_USE_AVX) | defined(NFE_USE_FMA)


#if defined(__LINUX__) | defined(__linux__)
#define _LARGEFILE64_SOURCE
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <linux/aio_abi.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>
#include <dirent.h>
#include <poll.h>
#include <pthread.h>
#endif // defined(__LINUX__) | defined(__linux__)

/// C lib
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>

/// STL
#include <unordered_map>    // TODO remove
#include <stack>            // TODO remove
#include <queue>            // TODO remove
#include <algorithm>

#include <sstream>          // TODO remove
#include <iostream>         // TODO remove
#include <iomanip>

#include <atomic>
#include <thread>

#include <memory>
#include <functional>
#include <limits>
#include <type_traits>
