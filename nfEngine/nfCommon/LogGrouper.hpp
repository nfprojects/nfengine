/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Declaration of LogGrouper class
 */

#pragma once

#include "nfCommon.hpp"
#include "Timer.hpp"
#include "Logger.hpp"

namespace NFE{
namespace Common{

class NFCOMMON_API LogGrouper
{
    struct LogStruct
    {
        LogType type;
        double time;
        int line;
        const char* msg;
    };

    const char* mFile;
    std::vector<LogStruct> mLogs;
    Timer mTimer;

    LogGrouper(const char* file);
    ~LogGrouper();
    void Log(LogType type, int line, const char* str, ...);
    void Log(LogType type, const char* str, int line);

    void Flush();
};

/// logging macros
#define LOG_GROUP_CREATE(name) LogGrouper #name(__FILE__);

#define LOG_ANY_GROUP(group, type, ...)                                 \
do {                                                                    \
    if (group)                                                          \
        group.Log(type, __LINE__, __VA_ARGS__);                        \
} while (0)

#define LOG_ANY_STREAM_GROUP(group, type, msg)                          \
do {                                                                    \
    if (group)                                                          \
    {                                                                   \
        std::stringstream stream;                                       \
        stream << msg;                                                  \
        group.Log(type, stream.str().c_str(), __LINE__);               \
    }                                                                   \
} while (0)

#ifdef _DEBUG
#define LOG_DEBUG_GROUP(group, ...)     LOG_ANY_GROUP(group, NFE::Common::LogType::Debug, __VA_ARGS__)
#define LOG_DEBUG_S_GROUP(group, msg)   LOG_ANY_STREAM_GROUP(group, NFE::Common::LogType::Debug, msg)
#else
#define LOG_DEBUG_GROUP(...)   do { } while (0)
#define LOG_DEBUG_S_GROUP(...) do { } while (0)
#endif // _DEBUG

#define LOG_INFO_GROUP(group, ...)      LOG_ANY_GROUP(group, NFE::Common::LogType::Info, __VA_ARGS__)
#define LOG_INFO_S_GROUP(group, msg)    LOG_ANY_STREAM_GROUP(group, NFE::Common::LogType::Info, msg)
#define LOG_SUCCESS_GROUP(group, ...)   LOG_ANY_GROUP(group, NFE::Common::LogType::OK, __VA_ARGS__)
#define LOG_SUCCESS_S_GROUP(group, msg) LOG_ANY_STREAM_GROUP(group, NFE::Common::LogType::OK, msg)
#define LOG_WARNING_GROUP(group, ...)   LOG_ANY_GROUP(group, NFE::Common::LogType::Warning, __VA_ARGS__)
#define LOG_WARNING_S_GROUP(group, msg) LOG_ANY_STREAM_GROUP(group, NFE::Common::LogType::Warning, msg)
#define LOG_ERROR_GROUP(group, ...)     LOG_ANY_GROUP(group, NFE::Common::LogType::Error, __VA_ARGS__)
#define LOG_ERROR_S_GROUP(group, msg)   LOG_ANY_STREAM_GROUP(group, NFE::Common::LogType::Error, msg)
#define LOG_FATAL_GROUP(...)            LOG_ANY_GROUP(group, NFE::Common::LogType::Fatal, __VA_ARGS__)
#define LOG_FATAL_S_GROUP(msg)          LOG_ANY_STREAM_GROUP(group, NFE::Common::LogType::Fatal, msg)


}
}