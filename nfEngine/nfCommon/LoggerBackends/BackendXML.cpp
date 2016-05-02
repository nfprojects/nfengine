/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of XML logger backend
 */

#include "../PCH.hpp"
#include "BackendXML.hpp"
#include "BackendCommon.hpp"
#include <array>


namespace NFE
{
namespace Common
{

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

    {
        // Declare all escaped chars and calculate their sizes once
        const char* escAmp = "&amp;";
        const unsigned char escAmpSize = static_cast<unsigned char>(strlen(escAmp));
        const char* escLt = "&lt;";
        const unsigned char escLtSize = static_cast<unsigned char>(strlen(escLt));
        const char* escGt = "&gt;";
        const unsigned char escGtSize = static_cast<unsigned char>(strlen(escGt));
        const char* escApos = "&apos;";
        const unsigned char escAposSize = static_cast<unsigned char>(strlen(escApos));
        const char* escQuot = "&quot;";
        const unsigned char escQuotSize = static_cast<unsigned char>(strlen(escQuot));

        // Escape from the log message all characters that need it
        for (const char* i = str; *i; i++)
            switch (*i)
            {
                case '&':
                    mBuffer.insert(mBuffer.begin() + outputStrSize, escAmp, escAmp + escAmpSize);
                    outputStrSize += escAmpSize;
                    break;
                case '<':
                    mBuffer.insert(mBuffer.begin() + outputStrSize, escLt, escLt + escLtSize);
                    outputStrSize += escLtSize;
                    break;
                case '>':
                    mBuffer.insert(mBuffer.begin() + outputStrSize, escGt, escGt + escGtSize);
                    outputStrSize += escGtSize;
                    break;
                case '\'':
                    mBuffer.insert(mBuffer.begin() + outputStrSize, escApos, escApos + escAposSize);
                    outputStrSize += escAposSize;
                    break;
                case '"':
                    mBuffer.insert(mBuffer.begin() + outputStrSize, escQuot, escQuot + escQuotSize);
                    outputStrSize += escQuotSize;
                    break;
                default:
                    mBuffer.insert(mBuffer.begin() + outputStrSize++, *i);
            }
    }

    // Enter closing node
    mBuffer.insert(mBuffer.begin() + outputStrSize++, '<');
    mBuffer.insert(mBuffer.begin() + outputStrSize++, '/');
    mBuffer.insert(mBuffer.begin() + outputStrSize, nodeName , nodeName + strlen(nodeName));
    outputStrSize += strlen(nodeName);
    mBuffer.insert(mBuffer.begin() + outputStrSize++, '>');
    mBuffer.insert(mBuffer.begin() + outputStrSize++, '\n');

    mFile.Write(mBuffer.data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
