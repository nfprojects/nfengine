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
#include "LoggerBackends/BackendTxt.hpp"
#include "LoggerBackends/BackendXML.hpp"
#include <stdarg.h>
#include <atomic>

#ifdef WIN32
#include "LoggerBackends/Win/BackendWindowsDebugger.hpp"
#include "Win/Common.hpp"
#endif // WIN32

namespace NFE {
namespace Common {

// global logger instance
Logger gLogger;

Logger::Logger()
    : mInitialized(false)
{
    mTimer.Start();

#ifdef WIN32
    wchar_t* wideRootDir = NFE_ROOT_DIRECTORY;
    UTF16ToUTF8(wideRootDir, mPathPrefix);
#else
    mPathPrefix = NFE_ROOT_DIRECTORY;
#endif
    mPathPrefixLen = mPathPrefix.length();

    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    mLogsDirectory = execDir + "/../../../Logs";
    FileSystem::CreateDir(mLogsDirectory);

    Reset();
}

Logger::~Logger()
{
}

Logger* Logger::GetInstance()
{
    /**
     * We can't use singleton here, because GetInstace() would be called recursively
     * (e.g. in FileSystem::CreateDir) and cause a deadlock.
     */
    return &gLogger;
}

void Logger::Reset()
{
    mInitialized = false;
    mBackends.clear();

    RegisterBackend(new LoggerBackendConsole);
    RegisterBackend(new LoggerBackendHTML);
    RegisterBackend(new LoggerBackendTxt);
    RegisterBackend(new LoggerBackendXML);
#ifdef WIN32
    if (IsDebuggerPresent())
        RegisterBackend(new LoggerBackendWinDebugger);
#endif // WIN32

    mInitialized = true;
    LogInit();
}

void Logger::LogInit()
{
    LOG_INFO("nfCommon build date: " __DATE__ ", " __TIME__);

#ifdef NFE_USE_SSE
    LOG_INFO("nfCommon built with SSE instructions");
#ifdef NFE_USE_SSE4
    LOG_INFO("nfCommon built with SSE4.1 instructions");
#endif // NFE_USE_SSE4
#else
    LOG_INFO("nfCommon built with native FPU instructions only");
#endif // NFE_USE_SSE

    LogRunTime();
}

void Logger::LogRunTime()
{
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

void Logger::RegisterBackend(LoggerBackend* backend)
{
    mBackends.emplace_back(std::unique_ptr<LoggerBackend>(backend));
}

void Logger::Log(LogType type, const char* srcFile, int line, const char* str, ...)
{
    if (!mInitialized)
        return;

    /// keep shorter strings on the stack for performance
    const int SHORT_MESSAGE_LENGTH = 1024;
    char stackBuffer[SHORT_MESSAGE_LENGTH];

    // TODO: consider logging local time instead of time elapsed since Logger initialization
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

    std::unique_lock<std::mutex> lock(mMutex);
    for (auto& backend : mBackends)
    {
        backend->Log(type, srcFile, line, formattedStr, logTime);
    }

    // If it's a fatal log, exit the engine
    if (type == NFE::Common::LogType::Fatal)
        exit(1);
}

void Logger::Log(LogType type, const char* srcFile, const char* msg, int line)
{
    if (!mInitialized)
        return;

    // TODO: consider logging local time instead of time elapsed since Logger initialization
    double logTime = mTimer.Stop();

    std::unique_lock<std::mutex> lock(mMutex);
    for (auto& backend : mBackends)
    {
        backend->Log(type, srcFile, line, msg, logTime);
    }

    // If it's a fatal log, exit the engine
    if (type == NFE::Common::LogType::Fatal)
        exit(1);
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
