/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Logger class
 */

#pragma once

#include "../nfCommon.hpp"

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
 * Main logger class.
 */
class NFCOMMON_API ILogger
{
    NFE_MAKE_NONCOPYABLE(ILogger)

public:
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
    virtual void Log(LogType type, const char* srcFile, int line, const char* str, ...) = 0;
    virtual void Log(LogType type, const char* srcFile, const char* str, int line) = 0;

    /**
     * Access logger singleton instance.
     */
    static ILogger* GetInstance();

protected:
    ILogger();
    virtual ~ILogger();
};


} // namespace Common
} // namespace NFE


// should be defined by the compiler as a path to the root engine's directory
#ifndef NFE_ROOT_DIRECTORY
    #define NFE_ROOT_DIRECTORY ""
    #pragma message("Warning: NFE_ROOT_DIRECTORY macro not defined")
#endif // NFE_ROOT_DIRECTORY


#define NFE_LOG_ANY(type, ...)                                          \
do {                                                                    \
    NFE::Common::ILogger* logger = NFE::Common::ILogger::GetInstance(); \
    if (logger) logger->Log(type,  __FILE__, __LINE__, __VA_ARGS__);    \
} while (0)


#define NFE_LOG_ANY_STREAM(type, msg)                                   \
do {                                                                    \
    NFE::Common::ILogger* logger = NFE::Common::ILogger::GetInstance(); \
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

