/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of LogGrouper class
 */

#include "PCH.hpp"
#include "LogGrouper.hpp"
#include "LoggerImpl.hpp"

#ifdef WIN32
#include "System/Win/Common.hpp"
#endif // WIN32


namespace NFE {
namespace Common {

const size_t LogGrouper::mInitialSize = 16;

LogGrouper::LogGrouper()
    : mLogs(new LogStruct[mInitialSize])
    , mCurrentSize(mInitialSize)
    , mLogCounter(0)
    , mTimer(Logger::GetInstance()->GetTimer())
{
}

LogGrouper::~LogGrouper()
{
    Flush();
}

void LogGrouper::Log(LogType type, int line, const char* file, const char* str, ...)
{
    if (Logger::ListBackends().Empty() || !Logger::IsInitialized())
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
        NFE_LOG_ERROR("vsnprintf() failed, format = \"%s\"", str);
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
    log.msg = String(formattedStr);

    InsertLog(log);

   // If it's a fatal log, flush logs
    if (type == NFE::Common::LogType::Fatal)
    {
        Flush();
    }
}

void LogGrouper::Log(LogType type, const char* str, int line, const char* file)
{
    if (Logger::ListBackends().Empty() || !Logger::IsInitialized())
        return;

    double logTime = mTimer.Stop();

    LogStruct log;
    log.time = logTime;
    log.line = line;
    log.file = file;
    log.type = type;
    log.msg = String(str);

    InsertLog(log);

    // If it's a fatal log, flush logs
    if (type == NFE::Common::LogType::Fatal)
    {
        Flush();
    }
}

void LogGrouper::Flush()
{
    const LoggerBackendMap& backends = Logger::ListBackends();
    for (size_t i = 0; i < mLogCounter; i++)
    {
        for (const auto& b : backends)
        {
            LogStruct log = mLogs[i];
            if (b.ptr->IsEnabled())
                b.ptr->Log(log.type, log.file.Str(), log.line, log.msg.Str(), log.time);
        }
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
        memcpy(newArray, mLogs.get(), sizeof(LogStruct) * mLogCounter);
        mLogs.reset(newArray);
    }

    mLogs[mLogCounter - 1] = log;
}

} // namespace Common
} // namespace NFE
