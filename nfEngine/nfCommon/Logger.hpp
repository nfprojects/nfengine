/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Logger class
 */

#pragma once

#include "nfCommon.hpp"
#include "Timer.hpp"

namespace NFE {
namespace Common {

enum class LogType
{
    Debug,
    Info,
    OK,
    Warning,
    Error,
    Fatal,
};

/**
* Logger backend interface.
*/
class NFCOMMON_API LoggerBackend
{
public:
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
    virtual void Log(LogType type, const char* srcFile, int line, const char* str,
                     double timeElapsed) = 0;
};


/**
 * Main logger class.
 */
class NFCOMMON_API Logger
{
private:
    std::mutex mMutex;
    Timer mTimer;
    std::vector<std::unique_ptr<LoggerBackend>> mBackends;

    Logger(const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;

public:
    Logger();
    ~Logger();

    void RegisterBackend(std::unique_ptr<LoggerBackend> backend);

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

    /**
     * Access logger singleton instance.
     */
    static Logger* GetInstance();

    /**
     * Translate log type to string.
     */
    static const char* LogTypeToString(LogType logType);
};

} // namespace Common
} // namespace NFE

// this macro should be defined globaly
#ifndef NFE_PROJECT_DIR
#define NFE_PROJECT_DIR
#endif // NFE_PROJECT_DIR

#ifndef NFE_ROOT_DIRECTORY
#define NFE_ROOT_DIRECTORY ""
#endif // NFE_ROOT_DIRECTORY

/// logging macros
#define LOG_ANY(type, ...)                                                           \
do {                                                                                 \
    NFE::Common::Logger* logger = NFE::Common::Logger::GetInstance();                \
    if (logger) logger->Log(type, NFE_PROJECT_DIR __FILE__, __LINE__, __VA_ARGS__);  \
} while (0)

#ifdef _DEBUG
#define LOG_DEBUG(...)   LOG_ANY(NFE::Common::LogType::Debug, __VA_ARGS__)
#else
#define LOG_DEBUG(...) do { } while (0)
#endif // _DEBUG

#define LOG_INFO(...)    LOG_ANY(NFE::Common::LogType::Info, __VA_ARGS__)
#define LOG_SUCCESS(...) LOG_ANY(NFE::Common::LogType::OK, __VA_ARGS__)
#define LOG_WARNING(...) LOG_ANY(NFE::Common::LogType::Warning, __VA_ARGS__)
#define LOG_ERROR(...)   LOG_ANY(NFE::Common::LogType::Error, __VA_ARGS__)
#define LOG_FATAL(...)   LOG_ANY(NFE::Common::LogType::Fatal, __VA_ARGS__)
