/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of ILoggerBackend class
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Containers/StringView.hpp"
#include "../Containers/UniquePtr.hpp"
#include "../Containers/StaticArray.hpp"


namespace NFE {
namespace Common {


/**
* Logger backend interface.
*/
class NFCOMMON_API ILoggerBackend
{
    NFE_MAKE_NONCOPYABLE(ILoggerBackend)

public:
    ILoggerBackend();
    virtual ~ILoggerBackend();

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

    /**
     * Initialize the backend. This will be called when user calls NFE::InitSubsystems().
     * All sorts of setups for given backend should happen here and afterwards, the Backend
     * should be prepared for logging.
     *
     * It is ensured that Log() will not be called before a successful Init() call.
     *
     * @return True if initialization was successful, false if backend failed to initialize.
     */
    virtual bool Init();

    NFE_INLINE void Enable(bool enable) { mIsEnabled = enable; }
    NFE_INLINE bool IsEnabled() const { return mIsEnabled; }

private:
    bool mIsEnabled;
};


// Typedefs to make these types shorter and more readable
using LoggerBackendPtr = UniquePtr<ILoggerBackend>;

struct LoggerBackendInfo
{
    StringView name;
    LoggerBackendPtr ptr;
};

using LoggerBackendMap = StaticArray<LoggerBackendInfo, 8>;


} // namespace Common
} // namespace NFE
