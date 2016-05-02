/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of XML logger backend
 */

#include "../PCH.hpp"
#include "BackendXML.hpp"
#include "BackendCommon.hpp"
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
     * backend can be easly customizable.
     */
    const static std::string gLogIntro = R"(<?xml version="1.0" encoding="utf-8"?>
<LoggerOutput title = "nfEngine - log file">
)";

    const std::string logFileName = "log.xml";
    mBuffer.resize(NFE_MAX_LOG_MESSAGE_LENGTH);

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

    const char* format = "\t<%s"
        " srcFile=\"%s\""
        " line=\"%i\""
        " secElapsed=\"%.4f\""
        ">";

    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();
    size_t len = snprintf(mBuffer.data(), mBuffer.size(), format,
             nodeName, srcFile + pathOffset,
             line, timeElapsed);

    // If file path is REALLY long, it may not fit
    size_t outputStrSize = len;
    if (outputStrSize >= mBuffer.size())
    {
        while (outputStrSize >= mBuffer.size())
            mBuffer.resize(2 * mBuffer.size());

        snprintf(mBuffer.data(), mBuffer.size(), format,
                 nodeName, srcFile + pathOffset,
                 line, timeElapsed);
    }

    // Escape from the log message all characters that need it
    for (const char* i = str; *i; i++)
        switch (*i)
        {
            case '&':
                strcpy(mBuffer.data() + outputStrSize, "&amp;");
                outputStrSize += 5;
                break;
            case '<':
                strcpy(mBuffer.data() + outputStrSize, "&lt;");
                outputStrSize += 4;
                break;
            case '>':
                strcpy(mBuffer.data() + outputStrSize, "&gt;");
                outputStrSize += 4;
                break;
            case '\'':
                strcpy(mBuffer.data() + outputStrSize, "&apos;");
                outputStrSize += 6;
                break;
            case '"':
                strcpy(mBuffer.data() + outputStrSize, "&quot;");
                outputStrSize += 6;
                break;
            default:
                mBuffer.insert(mBuffer.begin() + outputStrSize++, *i);
        }

    // Enter closing node
    strcpy(mBuffer.data() + outputStrSize, "</");
    outputStrSize += 2;
    mBuffer.insert(mBuffer.begin() + outputStrSize, nodeName , nodeName + strlen(nodeName));
    outputStrSize += strlen(nodeName);
    strcpy(mBuffer.data() + outputStrSize, ">\n");
    outputStrSize += 2;

    mFile.Write(mBuffer.data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
