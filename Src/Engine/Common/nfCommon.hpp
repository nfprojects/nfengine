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

// 'if constexpr' is a C++17 language extension
#pragma warning(disable: 4984)

// "C++ nonstandard extension: nameless struct"
#pragma warning(disable : 4201)

// "unreachable code" - in Final we can get a lot of those, due to use of __assume in assertion macro
#ifdef NFE_CONFIGURATION_FINAL
#pragma warning(disable : 4702)
#endif // NFE_CONFIGURATION_FINAL

#ifndef strdup
#define strdup _strdup
#endif

#endif // _MSC_VER


#if defined(__GNUC__)
    #define NFE_UNNAMED_STRUCT __extension__
#else
    #define NFE_UNNAMED_STRUCT
#endif


// detect CPU architecture
#if defined(_M_X64) || defined(__amd64__) || defined(_M_AMD64)
    #define NFE_ARCH_X64
#elif defined(_M_IX86) || defined(__i386__) || defined(__i386)
    #define NFE_ARCH_X86
#else
    #error "Target architecture not supported!"
#endif


// DLL import / export macro
#ifdef NFE_PLATFORM_WINDOWS
    #define NFE_API_EXPORT __declspec(dllexport)
    #define NFE_API_IMPORT __declspec(dllimport)
#else // NFE_PLATFORM_WINDOWS
    #define NFE_API_EXPORT __attribute__((visibility("default")))
    #define NFE_API_IMPORT __attribute__((visibility("default")))
#endif // NFE_PLATFORM_WINDOWS

// DLL import / export macro
#ifdef NFE_PLATFORM_WINDOWS
    #ifdef NFCOMMON_EXPORTS
        #define NFCOMMON_API NFE_API_EXPORT
    #else // NFCOMMON_EXPORTS
        #define NFCOMMON_API NFE_API_IMPORT
    #endif // NFCOMMON_EXPORTS
#else // NFE_PLATFORM_WINDOWS
    #define NFCOMMON_API __attribute__((visibility("default")))
#endif // NFE_PLATFORM_WINDOWS


// force global variable definition to be shared across all compilation units
#if defined(NFE_PLATFORM_WINDOWS)
    #define NFE_GLOBAL_CONST extern const __declspec(selectany)
#elif defined(__LINUX__) | defined(__linux__)
    #define NFE_GLOBAL_CONST const
#else
    #error Invalid platform
#endif


#define NFE_INLINE inline

// macro forcing a function to be inlined
#if defined(NFE_PLATFORM_LINUX)
    #define NFE_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(NFE_PLATFORM_WINDOWS)
    #define NFE_FORCE_INLINE __forceinline
#else
    #error Invalid platform
#endif


// macro forcing a function not to be inlined
#if defined(NFE_PLATFORM_WINDOWS)
    #define NFE_FORCE_NOINLINE __declspec(noinline)
#elif defined(NFE_PLATFORM_LINUX)
    #define NFE_FORCE_NOINLINE __attribute__((noinline))
#else
    #error Invalid platform
#endif


// aligning macro for objects using SIMD registers
#if defined(NFE_PLATFORM_WINDOWS)
    #define NFE_ALIGN(bytes) __declspec(align(bytes))
#elif defined(NFE_PLATFORM_LINUX)
    #define NFE_ALIGN(bytes) __attribute__((aligned(bytes)))
#else
    #error Invalid platform
#endif


#define NFE_CACHE_LINE_SIZE 64u


// macro for data prefetching from RAM to CPU cache.
#if defined(NFE_PLATFORM_WINDOWS)
    #ifdef NFE_USE_SSE
        #define NFE_PREFETCH_L1(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
    #else
        #define NFE_PREFETCH_L1(addr)
    #endif // NFE_USE_SSE
#elif defined(NFE_PLATFORM_LINUX)
    #define NFE_PREFETCH_L1(addr) __builtin_prefetch(addr, 0, 3) // prefetch for read to all cache levels
#endif // WIN32


// debug break
#if defined(NFE_PLATFORM_WINDOWS)
    #define NFE_BREAK() __debugbreak()
#elif defined(NFE_PLATFORM_LINUX)
    #define NFE_BREAK() __builtin_trap()
#else
    #error Invalid platform
#endif


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

// build bit flag
#define NFE_FLAG(x) (1<<(x))

namespace NFE {

// define basic data types
typedef uint64_t uint64;
typedef int64_t int64;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint8_t uint8;
typedef int8_t int8;


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


namespace Common {

/**
 * Safely initialize all NFE Common subsystems.
 * @remarks This should be called right after entering main(). Otherwise, some NFE Common modules
 *          might not work correctly.
 */
NFCOMMON_API bool InitSubsystems();

/**
 * Safely shutdown all the subsystems: RTTI, logger, memory, etc.
 * @remarks This must be called just before returning from main().
 *          Otherwise application will crash, because of non-deterministic global variables destruction.
 */
NFCOMMON_API void ShutdownSubsystems();


} // namespace Common
} // namespace NFE


#include "ForwardDeclarations.hpp"
