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

    const char* format = "\t<%s"
        " srcFile=\"%s\""
        " line=\"%i\""
        " secElapsed=\"%.4f\""
        ">";
        //%s</%s>\n";

    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();
    snprintf(mBuffer.data(), mBuffer.size(), format,
             nodeName.c_str(), srcFile + pathOffset,
             line, timeElapsed);

    for (const char* i = str; *i; i++)
    {
        switch (*i)
        {
            case '&':
                mBuffer.insert(mBuffer.end(), entityRefs[0], entityRefs[0] + strlen(entityRefs[0]));
                break;
            case '<':
                mBuffer.insert(mBuffer.end(), entityRefs[1], entityRefs[1] + strlen(entityRefs[1]));
                break;
            case '>':
                mBuffer.insert(mBuffer.end(), entityRefs[2], entityRefs[2] + strlen(entityRefs[2]));
                break;
            case '\'':
                mBuffer.insert(mBuffer.end(), entityRefs[3], entityRefs[3] + strlen(entityRefs[3]));
                break;
            case '"':
                mBuffer.insert(mBuffer.end(), entityRefs[4], entityRefs[4] + strlen(entityRefs[4]));
                break;
            default:
                mBuffer.push_back(*i);
        }
    }

    mBuffer.shrink_to_fit();
    mFile.Write(mBuffer.data(), mBuffer.size());
}

} // namespace Common
} // namespace NFE
