/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Base header for nfCommon project.
 */

#pragma once


// disable some Visual Studio specific warnings
#ifdef _MSC_VER
// "class 'type' needs to have dll-interface to be used by clients of class 'type2'"
#pragma warning(disable: 4251)

// "conditional expression is constant"
#pragma warning(disable: 4127)

// "interaction between '_setjmp' and C++ object destruction is non - portable"
#pragma warning(disable: 4611)
#endif


#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <map>
#include <atomic>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <thread>
#include <cstring>
#include <cassert>


// DLL import / export macro
#ifdef WIN32
#ifdef NFCOMMON_EXPORTS
#define NFCOMMON_API __declspec(dllexport)
#else // NFCOMMON_EXPORTS
#define NFCOMMON_API __declspec(dllimport)
#endif // NFCOMMON_EXPORTS
#else // WIN32
#define NFCOMMON_API __attribute__((visibility("default")))
#endif // WIN32

// __forceinline and declspec(align(x)) replacement on other systems

#if defined(__LINUX__) | defined(__linux__)
#define NFE_INLINE inline __attribute__((always_inline))
#elif defined(WIN32)
#define NFE_INLINE __forceinline
#endif // defined(__LINUX__) | defined(__linux__)

// 16-byte aligning macro for objects using SSE registers
#if defined(WIN32)
#define NFE_ALIGN16 __declspec(align(16))
#elif defined(__LINUX__) | defined(__linux__)
#define NFE_ALIGN16 __attribute__((aligned(16)))
#else
#error "Target system not supported!"
#endif // defined(WIN32)


namespace NFE {
namespace Common {

// Linux datatypes workaround
#if defined(__LINUX__) | defined(__linux__)
typedef uint64_t __uint64;
typedef int64_t __int64;
#elif defined(WIN32)
typedef unsigned __int64 __uint64;
#endif // defined(__LINUX__) | defined(__linux__)

// define basic data types
typedef __uint64 uint64;
typedef __int64 int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uchar;

} // namespace Common
} // namespace NFE
