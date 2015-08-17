/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of Logger class
 */

#include "PCH.hpp"
#include "Logger.hpp"
#include "FileSystem.hpp"
#include "LoggerBackends/BackendConsole.hpp"
#include "LoggerBackends/BackendHTML.hpp"
#include <stdarg.h>
#include <atomic>

namespace NFE {
namespace Common {

Logger::Logger()
{
}

Logger::~Logger()
{
}

Logger* Logger::GetInstance()
{
    static std::unique_ptr<Logger> gLogger;
    static std::atomic<bool> gLoggerInitialized(false);

    if (!gLoggerInitialized)
    {
        gLoggerInitialized = true;

        FileSystem::CreateDir("Logs");
        gLogger.reset(new (std::nothrow) Logger);
        if (!gLogger)
            return nullptr;

        gLogger->RegisterBackend(std::move(std::unique_ptr<LoggerBackend>(new LoggerBackendConsole)));
        gLogger->RegisterBackend(std::move(std::unique_ptr<LoggerBackend>(new LoggerBackendHTML)));
    }

    return gLogger.get();
}

void Logger::RegisterBackend(std::unique_ptr<LoggerBackend> backend)
{
    mBackends.push_back(std::move(backend));
}

void Logger::Log(LogType type, const char* function, const char* srcFile, int line,
                 const char* str, ...)
{
    /// keep shorter strings on the stack for performance
    const int SHORT_MESSAGE_LENGTH = 8;
    char buffer[SHORT_MESSAGE_LENGTH];

    double t = mTimer.Stop();
    char* formattedStr = buffer;

    va_list args;
    va_start(args, str);
    int len = vsnprintf(formattedStr, SHORT_MESSAGE_LENGTH, str, args);
    va_end(args);

    if (len < 0)
        return;
    else if (len >= SHORT_MESSAGE_LENGTH)
    {
        // buffer on the stack is too small
        formattedStr = new (std::nothrow) char [len + 1];
        if (formattedStr == nullptr)
            return;

        va_start(args, str);
        vsnprintf(formattedStr, len + 1, str, args);
        va_end(args);
    }

    {
        std::unique_lock<std::mutex> lock(mMutex);
        for (auto& backend : mBackends)
        {
            backend->Log(type, function, srcFile, line, formattedStr, t);
        }
    }

    if (len >= SHORT_MESSAGE_LENGTH)
        delete[] formattedStr;
}

const char* Logger::LogTypeToString(LogType logType)
{
    switch (logType)
    {
    case LogType::Info:
        return "INFO";
    case LogType::Success:
        return "SUCCESS";
    case LogType::Warning:
        return "WARNING";
    case LogType::Error:
        return "ERROR";
    case LogType::Fatal:
        return "FATAL";
    }

    return "UKNOWN";
}

} // namespace Common
} // namespace NFE
