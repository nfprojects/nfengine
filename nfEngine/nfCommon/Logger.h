/**
    NFEngine project

    \file   Logger.h
*/

#pragma once

#include "nfCommon.h"
#include "Timer.h"

namespace NFE {
namespace Common {

enum class LogType
{
    Info = 0x1,
    Success = 0x2,
    Warning = 0x4,
    Error = 0x8,
    Fatal = 0x10,

    All = 0x1F,
};

/**
 * Built-in logger output formats.
 */
enum LoggerOutputType
{
    Raw = 0,
    Html,
};

struct LoggerOutput
{
    std::ofstream file;
    LoggerOutputType outputType;
};

/**
 * Main logger class.
 */
class NFCOMMON_API Logger
{
private:
    std::mutex mMutex;
    Timer mTimer;
    std::map<const std::string, LoggerOutput*> mOutputs;

    Logger(const Logger&);
    Logger& operator= (const Logger&);

public:
    Logger();
    ~Logger();

    int OpenFile(const char* pFile, LoggerOutputType outputType);
    void CloseAll();

    /**
     * Log single line using formated string.
     * @param type Log level type
     * @param pFunction Function name
     * @param pSource Source file name
     * @param line Number of line in the source file
     * @param pStr Formated string
     * @remarks Use logging macros to simplify code
     */
    void Log(LogType type, const char* pFunction, const char* pSource, int line, const char* pStr, ...);

    /**
     * Access logger singletone instance.
     */
    static Logger* GetInstance();
};

} // namespace Common
} // namespace NFE


/// logging macros
#define LOG_INFO(msg, ...)      { if (Common::Logger::GetInstance()) Common::Logger::GetInstance()->Log(Common::LogType::Info, __FUNCTION__, __FILE__, __LINE__, (msg), __VA_ARGS__); }
#define LOG_SUCCESS(msg, ...)   { if (Common::Logger::GetInstance()) Common::Logger::GetInstance()->Log(Common::LogType::Success, __FUNCTION__, __FILE__, __LINE__, (msg), __VA_ARGS__); }
#define LOG_WARNING(msg, ...)   { if (Common::Logger::GetInstance()) Common::Logger::GetInstance()->Log(Common::LogType::Warning, __FUNCTION__, __FILE__, __LINE__, (msg), __VA_ARGS__); }
#define LOG_ERROR(msg, ...)     { if (Common::Logger::GetInstance()) Common::Logger::GetInstance()->Log(Common::LogType::Error, __FUNCTION__, __FILE__, __LINE__, (msg), __VA_ARGS__); }
#define LOG_FATAL(msg, ...)     { if (Common::Logger::GetInstance()) Common::Logger::GetInstance()->Log(Common::LogType::Fatal, __FUNCTION__, __FILE__, __LINE__, (msg), __VA_ARGS__); }
