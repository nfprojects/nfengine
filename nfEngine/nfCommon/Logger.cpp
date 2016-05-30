/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of Logger class
 */

#include "PCH.hpp"
#include "Logger.hpp"
#include "FileSystem.hpp"
#include "SystemInfo.hpp"
#include <stdarg.h>

#ifdef WIN32
#include "Win/Common.hpp"
#endif // WIN32


namespace NFE {
namespace Common {

Logger::Logger()
    : mInitialized(InitStage::Uninitialized)
{
}

Logger::~Logger()
{
}

Logger* Logger::GetInstance()
{
    static Logger mInstance;
    if (mInstance.mInitialized == InitStage::Uninitialized)
        std::call_once(mInstance.mInitFlag, &Logger::LogInit, mInstance);

    return &mInstance;
}

void Logger::Reset()
{
    mInitialized.store(InitStage::Initializing);

    for (const auto& i : mBackends())
    {
        i.second->Reset();
    }

    std::call_once(mInitFlag, &Logger::LogInit, this);
}

LoggerBackendMap& Logger::mBackends()
{
    static LoggerBackendMap mBackend;
    return mBackend;
}

void Logger::LogInit()
{
    mInitialized.store(InitStage::Initializing);
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

    mInitialized.store(InitStage::Initialized);

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
    LogSysInfo();
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

void Logger::LogSysInfo()
{
    SystemInfo& sysInfo = SystemInfo::Instance();

    LOG_INFO("CPU: %s, %u cores", sysInfo.GetCPUBrand().c_str(), sysInfo.GetCPUCoreNo());
    LOG_INFO("RAM: %uKB total, %uKB free", sysInfo.GetMemTotalPhysKb(), sysInfo.GetMemFreePhysKb());
    LOG_INFO("OS: %s", sysInfo.GetOSVersion().c_str());
    LOG_INFO("Compiler: %s", sysInfo.GetCompilerInfo().c_str());
}

bool Logger::RegisterBackend(const std::string& backendCode, LoggerBackend* backend)
{
    mBackends().emplace(backendCode, backend);
    return true;
}

LoggerBackend* Logger::GetBackend(const std::string& backendCode)
{
    LoggerBackendMap::const_iterator backend = mBackends().find(backendCode);

    if (backend == mBackends().end())
        return nullptr;

    return backend->second.get();
}

std::vector<std::string> Logger::ListBackends()
{
    std::vector<std::string> vect;
    vect.reserve(mBackends().size());

    for (const auto& i : mBackends())
        vect.push_back(i.first);

    return vect;
}

void Logger::Log(LogType type, const char* srcFile, int line, const char* str, ...)
{
    if (mBackends().empty() || mInitialized != InitStage::Initialized)
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
    for (const auto& backend : mBackends())
    {
        if (backend.second->IsEnabled())
            backend.second->Log(type, srcFile, line, formattedStr, logTime);
    }

    // If it's a fatal log, exit the engine
    if (type == NFE::Common::LogType::Fatal)
        exit(1);
}

void Logger::Log(LogType type, const char* srcFile, const char* msg, int line)
{
    if (mBackends().empty() || mInitialized != InitStage::Initialized)
        return;

    // TODO: consider logging local time instead of time elapsed since Logger initialization
    double logTime = mTimer.Stop();

    std::unique_lock<std::mutex> lock(mMutex);
    for (const auto& backend : mBackends())
    {
        if (backend.second->IsEnabled())
            backend.second->Log(type, srcFile, line, msg, logTime);
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
