/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of XML logger backend
 */

#include "../PCH.hpp"
#include "BackendXML.hpp"

#define NFE_MAX_LOG_MESSAGE_LENGTH 1024

/// snprintf was not implemented until Visual Studio 2015
#if defined(WIN32) && defined(_MSC_VER)
#if _MSC_VER <= 1800
#define snprintf sprintf_s
#endif
#endif

namespace NFE {
namespace Common {

LoggerBackendXML::LoggerBackendXML()
{
    /**
     * TODO: move intro, outro and the other XML code templates to another file, so the logger
     * backend can be easly customizable.
     */
    const static std::string gLogIntro = R"(<?xml version="1.0" encoding="utf-8"?>
<LoggerOutput title = "nfEngine - log file">
)";

    const std::string logFileName = "log.xml";
    const size_t initialBufferSize = 1024;
    mBuffer.resize(initialBufferSize);

    const std::string logFilePath = Logger::GetInstance()->GetLogsDirectory() + '/' + logFileName;
    if (!mFile.Open(logFilePath, AccessMode::Write, true))
    {
        // this will be handled by other logger
        LOG_ERROR("Failed to create XML log file");
        return;
    }

    mFile.Write(gLogIntro.data(), gLogIntro.length());
}

LoggerBackendXML::~LoggerBackendXML()
{
    const static std::string gLogOutro = R"(</LoggerOutput>)";

    mFile.Write(gLogOutro.data(), gLogOutro.length());
}

void LoggerBackendXML::Log(LogType type, const char* srcFile, int line, const char* str,
                            double timeElapsed)
{
    const char* format = "\t<Log type=\"%s\">\n"
        "\t\t<SecondsElapsed>%.4f</SecondsElapsed>\n"
        "\t\t<Message>%s</Message>\n"
        "\t\t<SourceFile>%s</SourceFile>\n"
        "\t\t<LineOfCode>%i</LineOfCode>\n"
        "\t</Log>\n";
                         
    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();
    int len = snprintf(mBuffer.data(), mBuffer.size(), format,
                       Logger::LogTypeToString(type), timeElapsed, str, srcFile + pathOffset, line);
    if (len < 0)
    {
        LOG_ERROR("snprintf() failed");
        return;
    }

    size_t outputStrSize = static_cast<size_t>(len);
    if (outputStrSize >= mBuffer.size())
    {
        while (outputStrSize >= mBuffer.size())
            mBuffer.resize(2 * mBuffer.size());

        snprintf(mBuffer.data(), mBuffer.size(), format,
                 Logger::LogTypeToString(type), timeElapsed, str, srcFile + pathOffset, line);
    }

    mFile.Write(mBuffer.data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
