/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Base header for nfCommon project.
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>


// disable some Visual Studio specific warnings
#ifdef _MSC_VER

// "class 'type' needs to have dll-interface to be used by clients of class 'type2'"
#pragma warning(disable: 4251)

// "conditional expression is constant"
#pragma warning(disable: 4127)

// "interaction between '_setjmp' and C++ object destruction is non - portable"
#pragma warning(disable: 4611)

// "structure was padded due to alignment specifier"
#pragma warning(disable: 4324)

// "C++ nonstandard extension: nameless struct"
#pragma warning(disable : 4201)

#ifndef strdup
#define strdup _strdup
#endif

#endif // _MSC_VER


// DLL import / export macro
#ifdef WIN32
#define NFE_API_EXPORT __declspec(dllexport)
#define NFE_API_IMPORT __declspec(dllimport)
#else // WIN32
#define NFE_API_EXPORT __attribute__((visibility("default")))
#define NFE_API_IMPORT __attribute__((visibility("default")))
#endif // WIN32

// DLL import / export macro
#ifdef WIN32
#ifdef NFCOMMON_EXPORTS
#define NFCOMMON_API NFE_API_EXPORT
#else // NFCOMMON_EXPORTS
#define NFCOMMON_API NFE_API_IMPORT
#endif // NFCOMMON_EXPORTS
#else // WIN32
#define NFCOMMON_API __attribute__((visibility("default")))
#endif // WIN32


// force global variable definition to be shared across all compilation units
#if defined(WIN32)
#define NFE_GLOBAL_CONST extern const __declspec(selectany)
#elif defined(__LINUX__) | defined(__linux__)
#define NFE_GLOBAL_CONST const
#else
#error "Target system not supported!"
#endif // defined(WIN32)


#define NFE_INLINE inline

// macro forcing a function to be inlined
#if defined(__LINUX__) | defined(__linux__)
#define NFE_INLINE inline __attribute__((always_inline))
#elif defined(WIN32)
#define NFE_FORCE_INLINE __forceinline
#endif // defined(__LINUX__) | defined(__linux__)


// macro forcing a function not to be inlined
#ifdef WIN32
#define NFE_FORCE_NOINLINE __declspec(noinline)
#elif defined(__LINUX__) || defined(__linux__)
#define NFE_FORCE_NOINLINE __attribute__((noinline))
#endif // WIN32


// aligning macro for objects using SIMD registers
#if defined(WIN32)
#define NFE_ALIGN(bytes) __declspec(align(bytes))
#elif defined(__LINUX__) | defined(__linux__)
#define NFE_ALIGN(bytes) __attribute__((aligned(bytes)))
#else
#error "Target system not supported!"
#endif // defined(WIN32)


#define NFE_CACHE_LINE_SIZE 64u


// macro for data prefetching from RAM to CPU cache.
#ifdef WIN32
#ifdef NFE_USE_SSE
#define NFE_PREFETCH_L1(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
#else
#define NFE_PREFETCH_L1(addr)
#endif // NFE_USE_SSE
#elif defined(__LINUX__) || defined(__linux__)
#define NFE_PREFETCH_L1(addr) __builtin_prefetch(addr, 0, 3) // prefetch for read to all cache levels
#endif // WIN32


// debug break
#if defined(WIN32)
#define NFE_BREAK() __debugbreak()
#elif defined(__LINUX__) | defined(__linux__)
#define NFE_BREAK() __builtin_trap()
#else
#error "Target system not supported!"
#endif // defined(WIN32)


// use this inside a class declaration to make it non-copyable
#define NFE_MAKE_NONCOPYABLE(Type)              \
    Type(const Type&) = delete;                 \
    Type& operator=(const Type&) = delete;


// use this inside a class declaration to make it non-moveable
#define NFE_MAKE_NONMOVEABLE(Type)              \
    Type(Type&&) = delete;                      \
    Type& operator=(Type&&) = delete;


// macro for disabling "unused parameter"
#ifndef NFE_UNUSED
#define NFE_UNUSED(x) (void)(x)
#endif // UNUSED

// merge two identifiers
#define NFE_MERGE(a, b) a##b

// generate unique name within a single compilation unit
#define NFE_UNIQUE_NAME_INTERNAL(prefix, line) NFE_MERGE(prefix, line)
#define NFE_UNIQUE_NAME(prefix) NFE_UNIQUE_NAME_INTERNAL(prefix, __LINE__)


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


// verify types sizes
static_assert(sizeof(uint8) == 1, "'uint8' type must have 1 byte");
static_assert(sizeof(int8) == 1, "'int8' type must have 1 byte");
static_assert(sizeof(uint16) == 2, "'uint16' type must have 2 bytes");
static_assert(sizeof(int16) == 2, "'int16' type must have 2 bytes");
static_assert(sizeof(float) == 4, "'float' type must have 4 bytes");
static_assert(sizeof(uint32) == 4, "'uint32' type must have 4 bytes");
static_assert(sizeof(int32) == 4, "'int32' type must have 4 bytes");
static_assert(sizeof(double) == 8, "'double' type must have 8 bytes");
static_assert(sizeof(uint64) == 8, "'uint64' type must have 8 bytes");
static_assert(sizeof(int64) == 8, "'int64' type must have 8 bytes");


// return size of an array
template<class T, size_t N>
size_t ArraySize(const T(&)[N])
{
    return N;
}


namespace Common {

/**
 * Safely shutdown all the subsystems: RTTI, logger, memory, etc.
 * @remarks This must be called just before returning from main().
 *          Otherwise application will crash, because of non-deterministic global variables destruction.
 */
NFCOMMON_API void ShutdownSubsystems();


} // namespace Common
} // namespace NFE


#include "ForwardDeclarations.hpp"
