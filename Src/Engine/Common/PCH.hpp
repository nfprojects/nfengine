/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Precompiled header.
 */

#pragma once

// enable memory allocation tracking (Windows only)
#if defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // defined(NFE_PLATFORM_WINDOWS) && defined(NFE_CONFIGURATION_DEBUG)

#if defined(NFE_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <VersionHelpers.h>
#endif // defined(NFE_PLATFORM_WINDOWS)


#ifdef NFE_USE_SSE
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#endif // NFE_USE_SSE


#if defined(NFE_PLATFORM_LINUX)
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
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
#endif // defined(NFE_PLATFORM_LINUX)

/// C lib
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>

/// STL
#include <typeinfo>
#include <string>
#include <unordered_map>    // TODO remove
#include <algorithm>

#include <sstream>          // TODO remove
#include <iostream>         // TODO remove
#include <iomanip>          // TODO remove

#include <atomic>

#include <memory>
#include <functional>
#include <limits>
#include <type_traits>
#include <initializer_list>
#include <iterator>
