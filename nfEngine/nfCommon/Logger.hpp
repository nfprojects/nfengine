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
    Info,
    Success,
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
     * @param type Log level type
     * @param srcFile Source file name
     * @param line Number of line in the source file
     * @param str Formated string
     * @remarks Use logging macros to simplify code
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

// this macro should be defined globaly by a project
#ifndef _PROJECT_DIR
#define _PROJECT_DIR
#endif

#if defined(WIN32)
#define PATH_SLASH "\\"
#elif defined(__LINUX__) | defined(__linux__)
#define PATH_SLASH "/"
#else
#error "Target system not supported!"
#endif

/// logging macros
#define LOG_ANY(type, ...)                                                                        \
do {                                                                                              \
    NFE::Common::Logger* logger = NFE::Common::Logger::GetInstance();                             \
    if (logger) logger->Log(type, _PROJECT_DIR PATH_SLASH __FILE__, __LINE__, __VA_ARGS__);       \
} while (0)

#define LOG_INFO(...)    LOG_ANY(NFE::Common::LogType::Info, __VA_ARGS__)
#define LOG_SUCCESS(...) LOG_ANY(NFE::Common::LogType::Success, __VA_ARGS__)
#define LOG_WARNING(...) LOG_ANY(NFE::Common::LogType::Warning, __VA_ARGS__)
#define LOG_ERROR(...)   LOG_ANY(NFE::Common::LogType::Error, __VA_ARGS__)
#define LOG_FATAL(...)   LOG_ANY(NFE::Common::LogType::Fatal, __VA_ARGS__)
