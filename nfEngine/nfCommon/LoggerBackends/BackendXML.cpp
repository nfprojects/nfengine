/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of XML logger backend
 */

#include "../PCH.hpp"
#include "BackendXML.hpp"
#include "BackendCommon.hpp"
#include <array>

namespace
{
std::array<const char*, 5> entityRefs = {
    "&amp;",
    "&lt;",
    "&gt;",
    "&apos;",
    "&quot;"
};
}

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

    const std::string closingNode = "</" + nodeName + ">\n";

    const char* format = "\t<%s"
        " srcFile=\"%s\""
        " line=\"%i\""
        " secElapsed=\"%.4f\""
        ">";

    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();
    size_t len = snprintf(mBuffer.data(), mBuffer.size(), format,
             nodeName.c_str(), srcFile + pathOffset,
             line, timeElapsed);

    // If file path is REALLY long, it may not fit
    size_t outputStrSize = len;
    if (outputStrSize >= mBuffer.size())
    {
        while (outputStrSize >= mBuffer.size())
            mBuffer.resize(2 * mBuffer.size());

        snprintf(mBuffer.data(), mBuffer.size(), format,
                 nodeName.c_str(), srcFile + pathOffset,
                 line, timeElapsed);
    }

    for (const char* i = str; *i; i++)
    {
        switch (*i)
        {
            case '&':
                len = strlen(entityRefs[0]);
                mBuffer.insert(mBuffer.begin() + outputStrSize, entityRefs[0],
                               entityRefs[0] + len);
                break;
            case '<':
                len = strlen(entityRefs[1]);
                mBuffer.insert(mBuffer.begin() + outputStrSize, entityRefs[1],
                               entityRefs[1] + std::char_traits<char>::length(entityRefs[1]));
                break;
            case '>':
                len = strlen(entityRefs[2]);
                mBuffer.insert(mBuffer.begin() + outputStrSize, entityRefs[2],
                               entityRefs[2] + strlen(entityRefs[2]));
                break;
            case '\'':
                len = strlen(entityRefs[3]);
                mBuffer.insert(mBuffer.begin() + outputStrSize, entityRefs[3],
                               entityRefs[3] + strlen(entityRefs[3]));
                break;
            case '"':
                len = strlen(entityRefs[4]);
                mBuffer.insert(mBuffer.begin() + outputStrSize, entityRefs[4],
                               entityRefs[4] + strlen(entityRefs[4]));
                break;
            default:
                len = 1;
                mBuffer.insert(mBuffer.begin() + outputStrSize, *i);
        }
        outputStrSize += len;
    }

    // Enter closing node
    mBuffer.insert(mBuffer.begin() + outputStrSize, closingNode.begin(), closingNode.end());
    outputStrSize += closingNode.size();

    mFile.Write(mBuffer.data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
