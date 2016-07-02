/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Basic types declarations. This file will be included by most of the engine's headers.
 */

#pragma once

#include "Prerequisites.hpp"


namespace NFE {

// DLL import / export macro
#ifdef NF_CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

//debugging
#ifndef NFE_ASSERT
#ifdef _DEBUG
#define NFE_ASSERT(e, s) { if ((e)==0) {OutputDebugString(s); DebugBreak();}}
#else
#define NFE_ASSERT(e, s)
#endif
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif // UNUSED

#define NFE_ALIGNED_CLASS(c) __declspec(align(16)) c : public Common::Aligned<16>

// define basic data types
typedef unsigned __int64 uint64;
typedef __int64 int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uchar;



// TODO: consider moving this to nfCommon
template<typename T>
class Rect
{
public:
    T Xmin, Xmax, Ymin, Ymax;

    Rect() : Xmin(0), Xmax(0), Ymin(0), Ymax(0) {};
    Rect(T Xmin_, T Ymin_, T Xmax_, T Ymax_) : Xmin(Xmin_), Xmax(Xmax_), Ymin(Ymin_), Ymax(Ymax_) {};
};

typedef Rect<int> Recti;
typedef Rect<float> Rectf;

/*
    Engine function and methods result. Replaces 'bool'.
*/
enum class Result
{
    OK = 0,

    //object is already initialized
    AlreadyInit,

    //object is already released
    AlreadyFree,

    //object is not initialized
    NotInitialized,

    //memory allocation error
    AllocationError,

    //pointer passed to a function points to invalid area (no access rights)
    CorruptedPointer,

    //could not open a file
    FileOpenError,

    InvalidFileFormat,

    UnsupportedPixelFormat,

    //some other error
    Error,

    //some other error
    MemoryCorruption,
};

} // namespace NFE
