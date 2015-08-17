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

#ifdef WIN32
#include "Win/Common.hpp"
#endif // WIN32

namespace NFE {
namespace Common {

Logger::Logger()
{
    mTimer.Start();

#ifdef WIN32
    wchar_t* wideRootDir = NFE_ROOT_DIRECTORY;
    UTF16ToUTF8(wideRootDir, mPathPrefix);
#else
    mPathPrefix = NFE_ROOT_DIRECTORY;
#endif

    mPathPrefixLen = mPathPrefix.length();
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
        gLoggerInitialized.store(true);

        FileSystem::CreateDir("Logs");
        gLogger.reset(new (std::nothrow) Logger);
        if (!gLogger)
            return nullptr;

        gLogger->RegisterBackend(std::move(std::unique_ptr<LoggerBackend>(new LoggerBackendConsole)));
        gLogger->RegisterBackend(std::move(std::unique_ptr<LoggerBackend>(new LoggerBackendHTML)));

        LOG_INFO("nfCommon build date: " __DATE__ ", " __TIME__);

        /// get current date and time
        time_t rawTime;
        time(&rawTime);
        struct tm* timeInfo = localtime(&rawTime);
        if (timeInfo != nullptr)
        {
            /// print current date and time, in format
            const int MAX_DATE_LENGTH = 32;
            char dateTimeStr[MAX_DATE_LENGTH];
            strftime(dateTimeStr, MAX_DATE_LENGTH, "%b %d %Y, %X", timeInfo);
            LOG_INFO("Run date: %s", dateTimeStr);
        }
    }

    return gLogger.get();
}

void Logger::RegisterBackend(std::unique_ptr<LoggerBackend> backend)
{
    mBackends.push_back(std::move(backend));
}

void Logger::Log(LogType type, const char* srcFile, int line, const char* str, ...)
{
    /// keep shorter strings on the stack for performance
    const int SHORT_MESSAGE_LENGTH = 1024;
    char buffer[SHORT_MESSAGE_LENGTH];

    double t = mTimer.Stop();

    // empty string
    if (str == nullptr)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        for (auto& backend : mBackends)
        {
            backend->Log(type, srcFile, line, "", t);
        }
        return;
    }

    char* formattedStr = buffer;
    va_list args, argsCopy;
    va_start(args, str);
    int len = vsnprintf(formattedStr, SHORT_MESSAGE_LENGTH, str, args);

    if (len < 0)
    {
        va_end(args);
        return;
    }
    else if (len >= SHORT_MESSAGE_LENGTH)
    {
        // buffer on the stack is too small
        formattedStr = new (std::nothrow) char[len + 1];
        if (formattedStr == nullptr)
            return;

        // we can't call vsnprintf with the same va_list more than once
        va_copy(argsCopy, args);
        vsnprintf(formattedStr, len + 1, str, argsCopy);
        va_end(argsCopy);
    }

    va_end(args);

    {
        std::unique_lock<std::mutex> lock(mMutex);
        for (auto& backend : mBackends)
        {
            backend->Log(type, srcFile, line, formattedStr, t);
        }
    }

    if (len >= SHORT_MESSAGE_LENGTH)
        delete[] formattedStr;
}

const char* Logger::LogTypeToString(LogType logType)
{
    switch (logType)
    {
    case LogType::Debug:
        return "DEBUG";
    case LogType::Info:
        return "INFO";
    case LogType::OK:
        return "SUCCESS";
    case LogType::Warning:
        return "WARNING";
    case LogType::Error:
        return "ERROR";
    case LogType::Fatal:
        return "FATAL";
    }

    return "UNKNOWN";
}

} // namespace Common
} // namespace NFE
