/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Logger class
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/Timer.hpp"
#include "../System/Mutex.hpp"
#include "../Containers/String.hpp" // TODO remove - this is circular dependency
#include "../Containers/UniquePtr.hpp"
#include "../Containers/StaticArray.hpp"

#include <atomic>


namespace NFE {
namespace Common {

enum class LogType
{
    Debug,
    Info,
    OK,      //< aka. Success
    Warning,
    Error,
    Fatal,
};


/**
* Logger backend interface.
*/
class NFCOMMON_API LoggerBackend
{
protected:
    bool mIsEnabled;

public:
    LoggerBackend()
        : mIsEnabled(true)
    {
    }

    virtual ~LoggerBackend()
    {
    }

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

    NFE_INLINE virtual void Reset() {};
    NFE_INLINE void Enable(bool enable) { mIsEnabled = enable; };
    NFE_INLINE bool IsEnabled() const { return mIsEnabled; };
};


// Typedefs to make these types shorter and more readable
using LoggerBackendPtr = UniquePtr<LoggerBackend>;

struct LoggerBackendInfo
{
    String name;
    LoggerBackendPtr ptr;
};

using LoggerBackendMap = StaticArray<LoggerBackendInfo, 8>;


/**
 * Main logger class.
 */
class NFCOMMON_API Logger
{
    enum class InitStage
    {
        Uninitialized,
        Initializing,
        Initialized,
    };

    String mLogsDirectory;

    /// for trimming source file paths in Log() method
    String mPathPrefix;
    uint32 mPathPrefixLen;

    std::atomic<InitStage> mInitialized;  //< Set to Initialized, when Logger is fully initialized
    Mutex mLogMutex;                 //< For synchronizing logger output
    Mutex mResetMutex;               //< For locking logger initialization
    Timer mTimer;
    static LoggerBackendMap& mBackends(); //< Method encapsulation to solve "static initialization order fiasco"

    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;

    void LogInit();
    void LogBuildInfo() const;
    void LogRunTime() const;
    void LogSysInfo() const;

public:
    /**
     * Register backend to be used for logging.
     *
     * @param name    Backend name.
     * @param backend Backend object - must implement LoggerBackend class.
     *
     * @return True, if new backend with @name was inserted. False if @name is already in use.
     */
    static bool RegisterBackend(const StringView name, LoggerBackendPtr backend);

    /**
     * Get pointer to already registered backend.
     *
     * @param name    Backend name.
     *
     * @return Pointer to the backend if registered, otherwise nullptr.
     */
    static LoggerBackend* GetBackend(const StringView name);

    /**
     * Get list of the registered backends.
     */
    static const LoggerBackendMap& ListBackends();

    /**
     * Log single line using formated string.
     * This function is thread-safe.
     *
     * @param type    Log level type
     * @param srcFile Source file name
     * @param line    Number of line in the source file
     * @param str     Formated string
     *
     * @remarks Use LOG_XXX macros to simplify code
     */
    void Log(LogType type, const char* srcFile, int line, const char* str, ...);
    void Log(LogType type, const char* srcFile, const char* str, int line);

    /**
     * Flush the log and destroy all the backends.
     */
    void Shutdown();

    /**
     * Reset all backends that save to a file
     */
    void Reset();

    /**
     * Get pre-calculated file path prefix (nfEngine root directory).
     */
    NFE_INLINE size_t GetPathPrefixLen() const
    {
        return mPathPrefixLen;
    }

    /**
     * Get logs directory location.
     */
    NFE_INLINE const String& GetLogsDirectory() const
    {
        return mLogsDirectory;
    }

    /**
     * Access logger singleton instance.
     */
    static Logger* GetInstance();

    /**
     * Translate log type to string.
     */
    static const char* LogTypeToString(LogType logType);

    /**
     * Check if Logger is fully initialized.
     */
    static bool IsInitialized();

    /**
     * Get current timer status.
     */
    const Timer& GetTimer() const;
};

} // namespace Common
} // namespace NFE

// should be defined by the compiler as a path to the root engine's directory
#ifndef NFE_ROOT_DIRECTORY
    #define NFE_ROOT_DIRECTORY ""
    #pragma message("Warning: NFE_ROOT_DIRECTORY macro not defined")
#endif // NFE_ROOT_DIRECTORY

/// logging macros
#define NFE_LOG_ANY(type, ...)                                              \
do {                                                                    \
    NFE::Common::Logger* logger = NFE::Common::Logger::GetInstance();   \
    if (logger) logger->Log(type,  __FILE__, __LINE__, __VA_ARGS__);    \
} while (0)

#define NFE_LOG_ANY_STREAM(type, msg)                                       \
do {                                                                    \
    NFE::Common::Logger* logger = NFE::Common::Logger::GetInstance();   \
    if (logger)                                                         \
    {                                                                   \
        std::stringstream stream;                                       \
        stream << msg;                                                  \
        logger->Log(type, __FILE__, stream.str().c_str(), __LINE__);    \
    }                                                                   \
} while (0)

#ifdef _DEBUG
#define NFE_LOG_DEBUG(...)     NFE_LOG_ANY(NFE::Common::LogType::Debug, __VA_ARGS__)
#define NFE_LOG_DEBUG_S(msg)   NFE_LOG_ANY_STREAM(NFE::Common::LogType::Debug, msg)
#else
#define NFE_LOG_DEBUG(...)   do { } while (0)
#define NFE_LOG_DEBUG_S(...) do { } while (0)
#endif // _DEBUG

#define NFE_LOG_INFO(...)      NFE_LOG_ANY(NFE::Common::LogType::Info, __VA_ARGS__)
#define NFE_LOG_INFO_S(msg)    NFE_LOG_ANY_STREAM(NFE::Common::LogType::Info, msg)
#define NFE_LOG_SUCCESS(...)   NFE_LOG_ANY(NFE::Common::LogType::OK, __VA_ARGS__)
#define NFE_LOG_SUCCESS_S(msg) NFE_LOG_ANY_STREAM(NFE::Common::LogType::OK, msg)
#define NFE_LOG_WARNING(...)   NFE_LOG_ANY(NFE::Common::LogType::Warning, __VA_ARGS__)
#define NFE_LOG_WARNING_S(msg) NFE_LOG_ANY_STREAM(NFE::Common::LogType::Warning, msg)
#define NFE_LOG_ERROR(...)     NFE_LOG_ANY(NFE::Common::LogType::Error, __VA_ARGS__)
#define NFE_LOG_ERROR_S(msg)   NFE_LOG_ANY_STREAM(NFE::Common::LogType::Error, msg)
#define NFE_LOG_FATAL(...)     NFE_LOG_ANY(NFE::Common::LogType::Fatal, __VA_ARGS__)
#define NFE_LOG_FATAL_S(msg)   NFE_LOG_ANY_STREAM(NFE::Common::LogType::Fatal, msg)

