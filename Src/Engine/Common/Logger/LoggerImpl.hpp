/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Logger class
 */

#pragma once

#include "Logger.hpp"
#include "LoggerBackend.hpp"
#include "../System/Timer.hpp"
#include "../System/Mutex.hpp"
#include "../Containers/String.hpp" // TODO remove - this is circular dependency
#include "../Containers/UniquePtr.hpp"

#include <atomic>


namespace NFE {
namespace Common {


/**
 * Main logger class.
 */
class NFCOMMON_API Logger : public ILogger
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

    void LogBuildInfo() const;
    void LogRunTime() const;
    void LogSysInfo() const;

    // Simplified logging routines used before Logger is fully loaded
    void EarlyLog(LogType type, const char* srcFile, int line, const char* str);

public:
    /**
     * Register backend to be used for logging.
     *
     * @param name    Backend name.
     * @param backend Backend object - must implement ILoggerBackend class.
     *
     * @return True, if new backend with @name was inserted. False if @name is already in use.
     */
    static bool RegisterBackend(const StringView name, LoggerBackendPtr&& backend);

    /**
     * Get pointer to already registered backend.
     *
     * @param name    Backend name.
     *
     * @return Pointer to the backend if registered, otherwise nullptr.
     */
    static ILoggerBackend* GetBackend(const StringView name);

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
     * Initialize Logger and its backends.
     */
    bool Init();

    /**
     * Flush the log and destroy all the backends.
     */
    void Shutdown();

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
