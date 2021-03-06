/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of raw txt logger backend
 */

#include "PCH.hpp"
#include "../LoggerImpl.hpp"
#include "BackendTxt.hpp"
#include "BackendCommon.hpp"
#include "../../Containers/StringView.hpp"
#include "../../System/Thread.hpp"


namespace NFE {
namespace Common {

// Register Txt backend
bool gLoggerBackendTxtRegistered = Logger::RegisterBackend(StringView("TXT"), MakeUniquePtr<LoggerBackendTxt>());

LoggerBackendTxt::LoggerBackendTxt()
{
}

bool LoggerBackendTxt::Init()
{
    const static StringView gLogIntro("nfEngine - log file\n"
        "[Seconds elapsed] [ThreadID] [LogType] [SourceFile]:[LineOfCode]: [Message]\n");

    const StringView logFileName("log.txt");
    mBuffer.Resize(NFE_MAX_LOG_MESSAGE_LENGTH);

    const String logFilePath = Logger::GetInstance()->GetLogsDirectory() + '/' + logFileName;
    if (!mFile.Open(logFilePath, AccessMode::Write, true))
    {
        // this will be handled by early print
        NFE_LOG_ERROR("Failed to create raw txt log file.");
        return false;
    }

    size_t written = mFile.Write(gLogIntro.Data(), gLogIntro.Length());
    if (written != gLogIntro.Length())
    {
        NFE_LOG_ERROR("Failed to write TXT backend's intro: wrote %zd bytes", written);
        return false;
    }

    return true;
}

void LoggerBackendTxt::Log(LogType type, const char* srcFile, int line, const char* str,
                            double timeElapsed)
{
    if (!mFile.IsOpened())
        return;

    const char* format = "%.4f {%u} [%-7s] %s:%i: %s\n";

    const uint32 currentThreadID = Thread::GetCurrentThreadID();

    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();

    int len = snprintf(mBuffer.Data(), mBuffer.Size(), format,
                       timeElapsed, currentThreadID, Logger::LogTypeToString(type),
                       srcFile + pathOffset, line, str);
    if (len < 0)
    {
        NFE_LOG_ERROR("snprintf() failed");
        return;
    }

    size_t outputStrSize = static_cast<size_t>(len);
    if (outputStrSize >= mBuffer.Size())
    {
        while (outputStrSize >= mBuffer.Size())
        {
            mBuffer.Resize(2 * mBuffer.Size());
        }

        snprintf(mBuffer.Data(), mBuffer.Size(), format,
                 timeElapsed, currentThreadID, Logger::LogTypeToString(type),
                 srcFile + pathOffset, line, str);
    }

    mFile.Write(mBuffer.Data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
