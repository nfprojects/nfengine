/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of XML logger backend
 */

#include "../PCH.hpp"
#include "BackendXML.hpp"
#include "BackendCommon.hpp"


namespace NFE{
namespace Common{

LoggerBackendXML::LoggerBackendXML()
    : mEntityRefs{
        {"&amp;",  '&'},
        {"&lt;",   '<'},
        {"&gt;",   '>'},
        {"&apos;", '\''},
        {"&quot;", '"'},
}
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
    std::string nodeName = "Log";

    switch (type)
    {
        case LogType::OK:
            nodeName += "Ok";
            break;
        case LogType::Warning:
            nodeName += "Warning";
            break;
        case LogType::Error:
            nodeName += "Error";
            break;
        case LogType::Fatal:
            nodeName += "Fatal";
            break;
        case LogType::Info:
            __fallthrough;
        default:
            nodeName += "Info";
            break;
    }

    const char* format = "\t<%s"
        " srcFile=\"%s\""
        " line=\"%i\""
        " secElapsed=\"%.4f\""
        ">%s</%s>\n";

    // Escape all required characters from the message
    std::string escapedString(str);
    size_t lastPos = 0;

    for (auto& i : mEntityRefs)
    {
        lastPos = 0;
        while (lastPos != std::string::npos)
        {
            lastPos = escapedString.find(i.second, lastPos);
            if (lastPos != std::string::npos)
            {
                escapedString.replace(lastPos, 1, i.first, 0, i.first.size());
                lastPos += i.first.size();
            }
        }
    }

    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();
    int len = snprintf(mBuffer.data(), mBuffer.size(), format,
                       nodeName.c_str(), srcFile + pathOffset,
                       line, timeElapsed, escapedString.c_str(), nodeName.c_str());
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
                 nodeName.c_str(), srcFile + pathOffset,
                 line, timeElapsed, escapedString.c_str(), nodeName.c_str());
    }

    mFile.Write(mBuffer.data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
