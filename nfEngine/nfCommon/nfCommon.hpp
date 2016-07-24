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

// macro forcing a function to be inlined
#if defined(__LINUX__) | defined(__linux__)
#define NFE_INLINE inline __attribute__((always_inline))
#elif defined(WIN32)
#define NFE_INLINE __forceinline
#endif // defined(__LINUX__) | defined(__linux__)

// macro forcing a function not to be inlined
#ifdef WIN32
#define NFE_NO_INLINE __declspec(noinline)
#elif defined(__LINUX__) || defined(__linux__)
#define NFE_NO_INLINE __attribute__((noinline))
#endif // WIN32

// 16-byte aligning macro for objects using SSE registers
#if defined(WIN32)
#define NFE_ALIGN16 __declspec(align(16))
#elif defined(__LINUX__) | defined(__linux__)
#define NFE_ALIGN16 __attribute__((aligned(16)))
#else
#error "Target system not supported!"
#endif // defined(WIN32)

// we assume that when using Windows we build for x86 CPU supporting SSE and SSE4
#if defined(WIN32)
#define NFE_USE_SSE
#define NFE_USE_SSE4
#endif // defined(WIN32)

// macro for data prefetching from RAM to CPU cache.
#ifdef WIN32
#ifdef NFE_USE_SSE
#define NFE_PREFETCH(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
#else
#define NFE_PREFETCH(addr)
#endif // NFE_USE_SSE
#elif defined(__LINUX__) || defined(__linux__)
#define NFE_PREFETCH(addr) __builtin_prefetch(addr, 0, 3) // prefetch for read to all cache levels
#endif // WIN32

// macro for disabling "unsused parameter"
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif // UNUSED

// debug assert
// TODO: reimplement
#ifndef NFE_ASSERT
#ifdef _DEBUG
#define NFE_ASSERT(e, s) { if ((e)==0) {OutputDebugStringA(s); DebugBreak();}}
#else
#define NFE_ASSERT(e, s)
#endif
#endif


namespace NFE {

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
typedef signed int int32;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned char uint8;
typedef signed char int8;

} // namespace NFE
