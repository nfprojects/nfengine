/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of LoggerBackend class
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Containers/String.hpp"
#include "../Containers/UniquePtr.hpp"
#include "../Containers/StaticArray.hpp"


namespace NFE {
namespace Common {


/**
* Logger backend interface.
*/
class NFCOMMON_API LoggerBackend
{
    NFE_MAKE_NONCOPYABLE(LoggerBackend)

public:
    LoggerBackend();
    virtual ~LoggerBackend();

    /**
     * Abstract function for logging messages. This function may be NOT thread-safe.
     *
     * @param type        Log level type
     * @param srcFile     Source file name
     * @param line        Number of line in the source file
     * @param str         Message string
     * @param timeElapsed Seconds elapsed since log beginning
     */
    virtual void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed) = 0;

    virtual void Reset();

    NFE_INLINE void Enable(bool enable) { mIsEnabled = enable; }
    NFE_INLINE bool IsEnabled() const { return mIsEnabled; }

private:
    bool mIsEnabled;
};


// Typedefs to make these types shorter and more readable
using LoggerBackendPtr = UniquePtr<LoggerBackend>;

struct LoggerBackendInfo
{
    String name;
    LoggerBackendPtr ptr;
};

using LoggerBackendMap = StaticArray<LoggerBackendInfo, 8>;


} // namespace Common
} // namespace NFE
