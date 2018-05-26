/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of XML logger backend
 */

#include "PCH.hpp"
#include "../LoggerImpl.hpp"
#include "BackendXML.hpp"
#include "BackendCommon.hpp"


namespace NFE {
namespace Common {

 // Register HTML backend
bool gLoggerBackendXMLRegistered = Logger::RegisterBackend(StringView("XML"), MakeUniquePtr<LoggerBackendXML>());

LoggerBackendXML::LoggerBackendXML()
{
    Reset();
}

void LoggerBackendXML::Reset()
{
    /**
     * TODO: move intro, outro and the other XML code templates to another file, so the logger
     * backend can be easily customizable.
     */
    const static StringView gLogIntro(R"(<?xml version="1.0" encoding="utf-8"?>
<LoggerOutput title = "nfEngine - log file">
)");

    const StringView logFileName("log.xml");
    mBuffer.Resize(NFE_MAX_LOG_MESSAGE_LENGTH);

    const String logFilePath = Logger::GetInstance()->GetLogsDirectory() + '/' + logFileName;
    if (!mFile.Open(logFilePath, AccessMode::Write, true))
    {
        // this will be handled by other logger
        NFE_LOG_ERROR("Failed to create XML log file");
        return;
    }

    mFile.Write(gLogIntro.Data(), gLogIntro.Length());
}

LoggerBackendXML::~LoggerBackendXML()
{
    const static String gLogOutro(R"(</LoggerOutput>)");

    mFile.Write(gLogOutro.Str(), gLogOutro.Length());
}

void LoggerBackendXML::Log(LogType type, const char* srcFile, int line, const char* str,
                           double timeElapsed)
{
    // XML Log node format
    const char* nodeName;

    switch (type)
    {
        case LogType::OK:
            nodeName = "LogOk";
            break;
        case LogType::Warning:
            nodeName = "LogWarning";
            break;
        case LogType::Error:
            nodeName = "LogError";
            break;
        case LogType::Fatal:
            nodeName = "LogFatal";
            break;
        case LogType::Info:
        default:
            nodeName = "LogInfo";
            break;
    }

    /**
     * TODO: using CDATA makes it possible to blow up everything, by logging "]]"
     */
    const char* format = "\t<%s"
        " srcFile=\"%s\""
        " line=\"%i\""
        " secElapsed=\"%.4f\">"
        "<![CDATA[%s]]>"
        "</%s>\n";

    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();
    int len = snprintf(mBuffer.Data(), mBuffer.Size(), format,
                       nodeName, srcFile + pathOffset, line,
                       timeElapsed, str, nodeName);

    if (len < 0)
    {
        NFE_LOG_ERROR("snprintf() failed");
        return;
    }

    size_t outputStrSize = static_cast<size_t>(len);
    if (outputStrSize >= mBuffer.Size())
    {
        while (outputStrSize >= mBuffer.Size())
            mBuffer.Resize(2 * mBuffer.Size());

        snprintf(mBuffer.Data(), mBuffer.Size(), format,
                 nodeName, srcFile + pathOffset, line,
                 timeElapsed, str, nodeName);
    }

    mFile.Write(mBuffer.Data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
