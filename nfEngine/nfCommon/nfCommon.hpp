/**
    NFEngine project

    \file   nfCommon.h
    \brief  Base header for nfCommon project.
*/

#pragma once

#pragma warning(disable: 4251)

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

// DLL import / export macro
#ifdef NFCOMMON_EXPORTS
#define NFCOMMON_API __declspec(dllexport)
#else
#define NFCOMMON_API __declspec(dllimport)
#endif

namespace NFE {
namespace Common {

// define basic data types
typedef unsigned __int64 uint64;
typedef __int64 int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uchar;

} // namespace Common
} // namespace NFE
