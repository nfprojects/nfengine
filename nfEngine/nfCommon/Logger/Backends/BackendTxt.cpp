/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of raw txt logger backend
 */

#include "PCH.hpp"
#include "BackendTxt.hpp"
#include "BackendCommon.hpp"


namespace NFE {
namespace Common {

// Register Txt backend
bool gLoggerBackendTxtRegistered = Logger::RegisterBackend("TXT", std::make_unique<LoggerBackendTxt>());

LoggerBackendTxt::LoggerBackendTxt()
{
    Reset();
}

void LoggerBackendTxt::Reset()
{
    const static std::string gLogIntro = "nfEngine - log file\n"
                                         "[Seconds elapsed] [LogType] "
                                         "[SourceFile]:[LineOfCode]: [Message]\n";

    const std::string logFileName = "log.txt";
    mBuffer.resize(NFE_MAX_LOG_MESSAGE_LENGTH);

    const std::string logFilePath = Logger::GetInstance()->GetLogsDirectory() + '/' + logFileName;
    if (!mFile.Open(logFilePath, AccessMode::Write, true))
    {
        // this will be handled by other logger
        NFE_LOG_ERROR("Failed to create raw txt log file.");
        return;
    }

    mFile.Write(gLogIntro.data(), gLogIntro.length());
}

void LoggerBackendTxt::Log(LogType type, const char* srcFile, int line, const char* str,
                            double timeElapsed)
{
    if (!mFile.IsOpened())
        return;

    const char* format = "%.4f [%-7s] %s:%i: %s\n";

    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();

    int len = snprintf(mBuffer.data(), mBuffer.size(), format,
                       timeElapsed, Logger::LogTypeToString(type),
                       srcFile + pathOffset, line, str);
    if (len < 0)
    {
        NFE_LOG_ERROR("snprintf() failed");
        return;
    }

    size_t outputStrSize = static_cast<size_t>(len);
    if (outputStrSize >= mBuffer.size())
    {
        while (outputStrSize >= mBuffer.size())
            mBuffer.resize(2 * mBuffer.size());

        snprintf(mBuffer.data(), mBuffer.size(), format,
                 timeElapsed, Logger::LogTypeToString(type),
                 srcFile + pathOffset, line, str);
    }

    mFile.Write(mBuffer.data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
