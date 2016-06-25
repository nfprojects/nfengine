/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of LogGrouper class
 */

#include "PCH.hpp"
#include "LogGrouper.hpp"
#include <stdarg.h>

#ifdef WIN32
#include "Win/Common.hpp"
#endif // WIN32

namespace NFE {
namespace Common {

LogGrouper::LogGrouper(const char* file)
    : mFile(file)
    , mTimer(Logger::GetInstance()->GetTimer())
{
}

LogGrouper::~LogGrouper()
{
    Flush();
}

void LogGrouper::Log(LogType type, int line, const char* str, ...)
{
    if (Logger::ListBackends().empty() || !Logger::IsInitialized())
        return;

    /// keep shorter strings on the stack for performance
    const int SHORT_MESSAGE_LENGTH = 1024;
    char stackBuffer[SHORT_MESSAGE_LENGTH];

    double logTime = mTimer.Stop();

    // empty string
    if (str == nullptr)
        return;

    std::unique_ptr<char[]> buffer;
    char* formattedStr = nullptr;
    va_list args, argsCopy;
    va_start(args, str);

    // we can't call vsnprintf with the same va_list more than once
    va_copy(argsCopy, args);

    int len = vsnprintf(stackBuffer, SHORT_MESSAGE_LENGTH, str, args);
    if (len < 0)
    {
        va_end(argsCopy);
        va_end(args);
        LOG_ERROR("vsnprintf() failed, format = \"%s\"", str);
        return;
    }

    if (len >= SHORT_MESSAGE_LENGTH)  // buffer on the stack is too small
    {
        buffer.reset(new (std::nothrow) char[len + 1]);
        if (buffer)
        {
            formattedStr = buffer.get();
            vsnprintf(formattedStr, len + 1, str, argsCopy);
        }
    }
    else if (len > 0)  // buffer on the stack is sufficient
        formattedStr = stackBuffer;

    va_end(argsCopy);
    va_end(args);

    if (len < 0 || !formattedStr)
        return;

    LogStruct log;
    log.time = logTime;
    log.line = line;
    log.type = type;
    log.msg = std::string(formattedStr);

    mLogs.push_back(log);

   // If it's a fatal log, flush logs and exit the engine
    if (type == NFE::Common::LogType::Fatal)
    {
        Flush();
        exit(1);
    }
}

void LogGrouper::Log(LogType type, const char* str, int line)
{
    if (Logger::ListBackends().empty() || !Logger::IsInitialized())
        return;

    double logTime = mTimer.Stop();

    LogStruct log;
    log.time = logTime;
    log.line = line;
    log.type = type;
    log.msg = std::string(str);

    mLogs.push_back(log);

    // If it's a fatal log, flush logs and exit the engine
    if (type == NFE::Common::LogType::Fatal)
    {
        Flush();
        exit(1);
    }
}

void LogGrouper::Flush()
{
    auto backends = Logger::ListBackends();
    for (const auto& i : mLogs)
        for (const auto& b : backends)
        {
            auto backend = Logger::GetBackend(b);
            if (backend->IsEnabled())
                backend->Log(i.type, mFile, i.line, i.msg.c_str(), i.time);
        }

    mLogs.clear();
}

} // namespace Common
} // namespace NFE
