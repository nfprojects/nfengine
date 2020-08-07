/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of Windows debugger logger backend
 */

#include "PCH.hpp"
#include "../../LoggerImpl.hpp"
#include "../../../System/Thread.hpp"
#include "BackendWindowsDebugger.hpp"


namespace NFE {
namespace Common {

// Register Windows debugger backend
bool gLoggerBackendWinDebuggerRegistered = IsDebuggerPresent() ?
                                           Logger::RegisterBackend(StringView("WinDebugger"),
                                                                   MakeUniquePtr<LoggerBackendWinDebugger>())
                                           : false;


LoggerBackendWinDebugger::LoggerBackendWinDebugger()
{
    const uint32 initialBufferSize = 1024;
    mBuffer.Resize(initialBufferSize);
}

void LoggerBackendWinDebugger::Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed)
{
    const char* format = "%s(%i): %.4f {%0X} [%s] %s\n";

    const uint32 currentThreadID = Thread::GetCurrentThreadID();

    int len = snprintf(mBuffer.Data(), mBuffer.Size(), format,
                       srcFile, line, timeElapsed, currentThreadID, Logger::LogTypeToString(type), str);
    if (len < 0)
    {
        NFE_LOG_ERROR("snprintf() failed");
        return;
    }

    size_t outputStrSize = static_cast<size_t>(len);
    if (outputStrSize >= mBuffer.Size()) // buffer is too small
    {
        while (outputStrSize >= mBuffer.Size())
            mBuffer.Resize(2 * mBuffer.Size());

        len = snprintf(mBuffer.Data(), mBuffer.Size(), format,
                       srcFile, line, timeElapsed, currentThreadID, Logger::LogTypeToString(type), str);
        if (len < 0)
        {
            NFE_LOG_ERROR("snprintf() failed");
            return;
        }
    }

    mDebugString = mBuffer.Data();
    if (!UTF8ToUTF16(mDebugString, mWideDebugString))
        return;

    OutputDebugStringW(mWideDebugString.c_str());
}

} // namespace Common
} // namespace NFE
