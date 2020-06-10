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
#include "System/Windows/Common.hpp"
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
    return &mInstance;
}

void Logger::Shutdown()
{
    if (mInitialized != InitStage::Initialized)
        return; // already shut down

    NFE_SCOPED_LOCK(mResetMutex);

    mInitialized.store(InitStage::Uninitialized);

    mBackends().Clear();
}

LoggerBackendMap& Logger::mBackends()
{
    static LoggerBackendMap mBackend;
    return mBackend;
}

bool Logger::Init()
{
    if (mInitialized == InitStage::Initialized)
    {
        NFE_LOG_ERROR("Logger already initialized!");
        return false;
    }

    // Transition Logger state to mark the beginning of initialization
    mInitialized.store(InitStage::Initializing);

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
    if (!FileSystem::CreateDirIfNotExist(mLogsDirectory))
    {
        NFE_LOG_ERROR("Failed to create a directory for logs");
        return false;
    }

    // Initialize all backends, so they create log files in the Logs dir
    {
        NFE_SCOPED_LOCK(mResetMutex);

        // Initialize backends - not all have to successfully initialize.
        // Some backends will always initialize (ex. Console, WindowsDebugger).
        // If more sopthisticated backends fail, these will always serve as backup.
        for (const auto& backend: mBackends())
        {
            if (!backend.ptr->Init())
            {
                // TODO it might be a good idea to purge disabled backends here or later on
                NFE_LOG_ERROR("%s logger backend failed to initialize", backend.name.Str());
                backend.ptr->Enable(false);
            }
            else
            {
                NFE_LOG_INFO("%s logger backend enabled", backend.name.Str());
                backend.ptr->Enable(true);
            }
        }

        mTimer.Start();

        // Backends are done initializing - allow full logging
        mInitialized.store(InitStage::Initialized);

        // Print initial info
        NFE_LOG_INFO("Logger initialized");
        LogBuildInfo();
        LogRunTime();
        LogSysInfo();
    }

    return true;
}

void Logger::LogBuildInfo() const
{
    NFE_LOG_INFO("Compiler: %s", SystemInfo::Instance().GetCompilerInfo().Str());
    NFE_LOG_INFO("nfCommon build date: " __DATE__ ", " __TIME__);

    DynArray<const char*> instructionSet;

#ifdef NFE_USE_SSE
    instructionSet.PushBack("SSE");
#endif // NFE_USE_SSE

#ifdef NFE_USE_AVX
    instructionSet.PushBack("AVX");
#endif // NFE_USE_AVX

#ifdef NFE_USE_AVX2
    instructionSet.PushBack("AVX2");
#endif // NFE_USE_AVX

#ifdef NFE_USE_AVX512
    instructionSet.PushBack("AVX-512");
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

void Logger::EarlyLog(LogType type, const char* srcFile, int line, const char* str)
{
    printf("EARLY  [%-7s] %s:%i: %s\n", LogTypeToString(type), srcFile, line, str);
}

void Logger::Log(LogType type, const char* srcFile, int line, const char* str, ...)
{
    /// keep shorter strings on the stack for performance
    const int SHORT_MESSAGE_LENGTH = 1024;
    char stackBuffer[SHORT_MESSAGE_LENGTH];

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

    if (mBackends().Empty() || mInitialized != InitStage::Initialized)
    {
        // early log and exit since backends are not yet ready
        EarlyLog(type, srcFile, line, formattedStr);
        return;
    }

    // TODO: consider logging local time instead of time elapsed since Logger initialization
    double logTime = mTimer.Stop();

    NFE_SCOPED_LOCK(mLogMutex);
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
    {
        // early log and exit since backends are not yet ready
        EarlyLog(type, srcFile, line, str);
        return;
    }

    // TODO: consider logging local time instead of time elapsed since Logger initialization
    double logTime = mTimer.Stop();

    NFE_SCOPED_LOCK(mLogMutex);
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
