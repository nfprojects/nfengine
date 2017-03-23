/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of XML logger backend
 */

#include "PCH.hpp"
#include "BackendXML.hpp"
#include "BackendCommon.hpp"
#include "Containers/String.hpp"

#include <array>


namespace NFE {
namespace Common {

 // Register HTML backend
bool gLoggerBackendXMLRegistered = Logger::RegisterBackend("XML", std::make_unique<LoggerBackendXML>());

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
    const static String gLogIntro = R"(<?xml version="1.0" encoding="utf-8"?>
<LoggerOutput title = "nfEngine - log file">
)";

    const String logFileName = "log.xml";
    mBuffer.resize(NFE_MAX_LOG_MESSAGE_LENGTH);

    const String logFilePath = Logger::GetInstance()->GetLogsDirectory() + '/' + logFileName;
    if (!mFile.Open(logFilePath, AccessMode::Write, true))
    {
        // this will be handled by other logger
        LOG_ERROR("Failed to create XML log file");
        return;
    }

    mFile.Write(gLogIntro.Str(), gLogIntro.Length());
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
    int len = snprintf(mBuffer.data(), mBuffer.size(), format,
                       nodeName, srcFile + pathOffset, line,
                       timeElapsed, str, nodeName);

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
                 nodeName, srcFile + pathOffset, line,
                 timeElapsed, str, nodeName);
    }

    mFile.Write(mBuffer.data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
