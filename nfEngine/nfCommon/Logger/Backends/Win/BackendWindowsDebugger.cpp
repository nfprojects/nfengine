/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of Windows debugger logger backend
 */

#include "PCH.hpp"
#include "BackendWindowsDebugger.hpp"
#include "System/Win/Common.hpp"


namespace NFE {
namespace Common {

// Register Windows debugger backend
bool gLoggerBackendWinDebuggerRegistered = IsDebuggerPresent() ?
                                           Logger::RegisterBackend("WinDebugger",
                                                                   std::make_unique<LoggerBackendWinDebugger>())
                                           : false;


LoggerBackendWinDebugger::LoggerBackendWinDebugger()
{
    const size_t initialBufferSize = 1024;
    mBuffer.resize(initialBufferSize);
}

void LoggerBackendWinDebugger::Log(LogType type, const char* srcFile, int line, const char* str,
                                   double timeElapsed)
{
    const char* format = "%s(%i): %.4f [%s] %s\n";
    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();

    int len = snprintf(mBuffer.data(), mBuffer.size(), format,
                       srcFile + pathOffset, line, timeElapsed, Logger::LogTypeToString(type), str);
    if (len < 0)
    {
        LOG_ERROR("snprintf() failed");
        return;
    }

    size_t outputStrSize = static_cast<size_t>(len);
    if (outputStrSize >= mBuffer.size()) // buffer is too small
    {
        while (outputStrSize >= mBuffer.size())
            mBuffer.resize(2 * mBuffer.size());

        len = snprintf(mBuffer.data(), mBuffer.size(), format,
                       srcFile + pathOffset, line, timeElapsed, Logger::LogTypeToString(type), str);
        if (len < 0)
        {
            LOG_ERROR("snprintf() failed");
            return;
        }
    }

    mDebugString = mBuffer.data();
    if (!UTF8ToUTF16(mDebugString, mWideDebugString))
        return;

    OutputDebugStringW(mWideDebugString.c_str());
}

} // namespace Common
} // namespace NFE
