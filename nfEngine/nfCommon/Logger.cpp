/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of Logger class
 */

#include "PCH.hpp"
#include "Logger.hpp"
#include "FileSystem.hpp"
#include "LoggerBackends/BackendConsole.hpp"
#include <stdarg.h>

#define NFE_MAX_LOG_MESSAGE_LENGTH 1024

namespace NFE {
namespace Common {

Logger::Logger()
{
}

Logger::~Logger()
{
}

Logger* InitStaticLogger()
{
    FileSystem::CreateDir("Logs", true);
    Logger* logger = new Logger;
    if (!logger)
        return nullptr;

    std::unique_ptr<LoggerBackend> defaultBackend (new LoggerBackendConsole);
    logger->RegisterBackend(std::move(defaultBackend));

    return logger;
}

Logger* Logger::GetInstance()
{
    static std::unique_ptr<Logger> s_Logger(InitStaticLogger());
    return s_Logger.get();
}

void Logger::RegisterBackend(std::unique_ptr<LoggerBackend> backend)
{
    mBackends.push_back(std::move(backend));
}

void Logger::Log(LogType type, const char* function, const char* srcFile, int line,
                 const char* str, ...)
{
    double t = mTimer.Stop();

    char* formattedStr;
    char buffer[NFE_MAX_LOG_MESSAGE_LENGTH];
    va_list args;
    va_start(args, str);

    int len = vsnprintf(nullptr, 0, str, args);

    if (len < 0)
        assert(0);
    else if (len < NFE_MAX_LOG_MESSAGE_LENGTH)
        formattedStr = buffer;
    else
    {
        // buffer on the stack is too small
        formattedStr = new (std::nothrow) char [len];
        if (formattedStr == nullptr)
            return;
    }

    vsnprintf(formattedStr, len, str, args);

    {
        std::unique_lock<std::mutex> lock(mMutex);
        for (auto& backend : mBackends)
        {
            backend->Log(type, function, srcFile, line, formattedStr, t);
        }
    }

    if (len >= NFE_MAX_LOG_MESSAGE_LENGTH)
        delete[] formattedStr;

    va_end(args);
}

} // namespace Common
} // namespace NFE
