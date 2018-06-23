/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of Logger class
 */

#include "PCH.hpp"
#include "LoggerImpl.hpp"
#include "LoggerBackend.hpp"
#include "FileSystem/FileSystem.hpp"
#include "System/SystemInfo.hpp"
#include "Utils/ScopedLock.hpp"
#include "Containers/DynArray.hpp"

#ifdef WIN32
#include "System/Win/Common.hpp"
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

    // Double check, because we don't want uninit variable and compare_exchange every GetInstance call
    if (mInstance.mInitialized != InitStage::Initialized)
    {
        InitStage uninit = InitStage::Uninitialized;
        if (mInstance.mInitialized.compare_exchange_strong(uninit, InitStage::Initializing))
            mInstance.LogInit();
    }
    return &mInstance;
}

void Logger::Shutdown()
{
    ScopedExclusiveLock<Mutex> lock(mResetMutex);

    mBackends().Clear();
}

void Logger::Reset()
{
    if (mInitialized == InitStage::Uninitialized)
        return;

    ScopedExclusiveLock<Mutex> lock(mResetMutex);
    // Change mInitialized, to avoid Logging while backends are resetting
    mInitialized.store(InitStage::Initializing);

    mBackends().Clear();

    // Backends are done resetting - allow logging
    mInitialized.store(InitStage::Initialized);

    LogBuildInfo();
    LogRunTime();
    LogSysInfo();
}

LoggerBackendMap& Logger::mBackends()
{
    static LoggerBackendMap mBackend;
    return mBackend;
}

void Logger::LogInit()
{
    mTimer.Start();

#ifdef WIN32
    wchar_t* wideRootDir = NFE_ROOT_DIRECTORY;
    UTF16ToUTF8(wideRootDir, mPathPrefix);
#else
    mPathPrefix = NFE_ROOT_DIRECTORY;
#endif
    mPathPrefixLen = mPathPrefix.Length();

    String execPath = NFE::Common::FileSystem::GetExecutablePath();
    String execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    mLogsDirectory = execDir + "/../../../Logs";
    FileSystem::CreateDir(mLogsDirectory);

    // Reset all backends, so they recreate log files now, that the Logs dir is created.
    // Reset() methods will change mInitialize to Initialized state after resetting backends.
    Reset();
}

void Logger::LogBuildInfo() const
{
    NFE_LOG_INFO("Compiler: %s", SystemInfo::Instance().GetCompilerInfo().Str());
    NFE_LOG_INFO("nfCommon build date: " __DATE__ ", " __TIME__);

    DynArray<const char*> instructionSet;

#ifdef NFE_USE_SSE
    instructionSet.PushBack("SSE");
#endif // NFE_USE_SSE

#ifdef NFE_USE_SSE4
    instructionSet.PushBack("SSE4");
#endif // NFE_USE_SSE4

#ifdef NFE_USE_AVX
    instructionSet.PushBack("AVX");
#endif // NFE_USE_AVX

#ifdef NFE_USE_FMA
    instructionSet.PushBack("FMA");
#endif // NFE_USE_FMA

    if (instructionSet.Empty())
    {
        NFE_LOG_INFO("nfCommon built with native FPU instructions only");
    }
    else
    {
        String str;
        for (const char* name : instructionSet)
        {
            str += ' ';
            str += name;
        }
        NFE_LOG_INFO("nfCommon built with instructions:%s", str.Str());
    }
}

void Logger::LogRunTime() const
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
        NFE_LOG_INFO("Run date: %s", dateTimeStr);
    }
}

void Logger::LogSysInfo() const
{
    SystemInfo& sysInfo = SystemInfo::Instance();

    NFE_LOG_INFO("CPU: %s, %u cores", sysInfo.GetCPUBrand().Str(), sysInfo.GetCPUCoreNo());
    NFE_LOG_INFO("RAM: %uKB total, %uKB free", sysInfo.GetMemTotalPhysKb(), sysInfo.GetMemFreePhysKb());
    NFE_LOG_INFO("OS: %s", sysInfo.GetOSVersion().Str());
}

bool Logger::RegisterBackend(const StringView name, LoggerBackendPtr&& backend)
{
    for (const auto& backendInfo : mBackends())
    {
        if (backendInfo.name == name)
        {
            return false;
        }
    }

    LoggerBackendInfo info = { name, std::move( backend ) };
    mBackends().PushBack(std::move(info));
    return true;
}

ILoggerBackend* Logger::GetBackend(const StringView name)
{
    for (const auto& backend : mBackends())
    {
        if (backend.name == name)
        {
            return backend.ptr.Get();
        }
    }

    return nullptr;
}

const LoggerBackendMap& Logger::ListBackends()
{
    return mBackends();
}

void Logger::Log(LogType type, const char* srcFile, int line, const char* str, ...)
{
    if (mBackends().Empty() || mInitialized != InitStage::Initialized)
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

    ScopedExclusiveLock<Mutex> lock(mLogMutex);
    for (const auto& backend : mBackends())
    {
        if (backend.ptr->IsEnabled())
        {
            backend.ptr->Log(type, srcFile, line, formattedStr, logTime);
        }
    }
}

void Logger::Log(LogType type, const char* srcFile, const char* str, int line)
{
    if (mBackends().Empty() || mInitialized != InitStage::Initialized)
        return;

    // TODO: consider logging local time instead of time elapsed since Logger initialization
    double logTime = mTimer.Stop();

    ScopedExclusiveLock<Mutex> lock(mLogMutex);
    for (const auto& backend : mBackends())
    {
        if (backend.ptr->IsEnabled())
        {
            backend.ptr->Log(type, srcFile, line, str, logTime);
        }
    }
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

bool Logger::IsInitialized()
{
    return Logger::GetInstance()->mInitialized == Logger::InitStage::Initialized;
}

const Timer& Logger::GetTimer() const
{
    return mTimer;
}

} // namespace Common
} // namespace NFE
