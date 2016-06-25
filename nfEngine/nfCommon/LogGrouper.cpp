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

const size_t LogGrouper::mInitialSize = 16;

LogGrouper::LogGrouper()
    : mTimer(Logger::GetInstance()->GetTimer())
    , mLogs(new LogStruct[mInitialSize])
    , mCurrentSize(mInitialSize)
    , mLogCounter(0)
{
}

LogGrouper::~LogGrouper()
{
    Flush();
}

void LogGrouper::Log(LogType type, int line, const char* file, const char* str, ...)
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
    log.file = file;
    log.type = type;
    log.msg = std::string(formattedStr);

    InsertLog(log);

   // If it's a fatal log, flush logs and exit the engine
    if (type == NFE::Common::LogType::Fatal)
    {
        Flush();
        exit(1);
    }
}

void LogGrouper::Log(LogType type, const char* str, int line, const char* file)
{
    if (Logger::ListBackends().empty() || !Logger::IsInitialized())
        return;

    double logTime = mTimer.Stop();

    LogStruct log;
    log.time = logTime;
    log.line = line;
    log.file = file;
    log.type = type;
    log.msg = std::string(str);

    InsertLog(log);

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
    for (int i = 0; i < mLogCounter; i++)
        for (const auto& b : backends)
        {
            auto backend = Logger::GetBackend(b);
            LogStruct log = mLogs[i];
            if (backend->IsEnabled())
                backend->Log(log.type, log.file.c_str(), log.line, log.msg.c_str(), log.time);
        }

    mLogCounter = 0;
}

void LogGrouper::InsertLog(LogStruct log)
{
    mLogCounter++;
    if (mCurrentSize == mLogCounter)
    {
        mCurrentSize *= 2;
        LogStruct* newArray = new LogStruct[mCurrentSize];
        memcpy(newArray, mLogs, sizeof(LogStruct) * mLogCounter);
        delete[] mLogs;
        mLogs = newArray;
    }

    mLogs[mLogCounter - 1] = log;
}

} // namespace Common
} // namespace NFE
