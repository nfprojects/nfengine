/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of HTML logger backend
 */

#include "../PCH.hpp"
#include "BackendHTML.hpp"
#include "BackendCommon.hpp"

namespace NFE {
namespace Common {

// Register HTML backend
bool gLoggerBackendHTMLRegistered = Logger::RegisterBackend("HTML", std::make_unique<LoggerBackendHTML>());

LoggerBackendHTML::LoggerBackendHTML()
{
    Reset();
}

void LoggerBackendHTML::Reset()
{
    /**
     * TODO: move intro, outro and the other HTML code templates to another file, so the logger
     * backend can be easly customizable.
     */
    const static std::string gLogIntro = R"(
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html>
    <head>
        <title>nfEngine log</title>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
        <link rel="stylesheet" type="text/css" href="../nfEngine/Data/nfEngineLogs.css">
        <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.1.0/jquery.min.js"></script>
    </head>
    <body>
       <h3>nfEngine - log file</h3>
       <br/>

       <table id='log'>
       <col width='7%' valign='top'>
       <col width='74%' valign='top'>
       <col width='19%' valign='top'>
       <thead>
            <tr>
                <th>
                    <input type="text" id="search_time" class="search" placeholder="Search..."
                </th>
                <th>
                    <input type="text" id="search_message" class="search" placeholder="Search...">
                </th>
                <th>
                    <nobr>
                        <input type="text" id="search_srcFile" class="search" placeholder="Search...">
                    </nobr>
                </th>
            </tr>
            <tr>
               <th>Seconds elapsed</th>
               <th>Message ...</th>
               <th><nobr>SourceFile : LineOfCode</nobr></th>
           </tr>
       </thead>
       <tbody>
)";

    const std::string logFileName = "log.html";
    mBuffer.resize(NFE_MAX_LOG_MESSAGE_LENGTH);

    const std::string logFilePath = Logger::GetInstance()->GetLogsDirectory() + '/' + logFileName;
    if (!mFile.Open(logFilePath, AccessMode::Write, true))
    {
        // this will be handled by other logger
        LOG_ERROR("Failed to create HTML log file");
        return;
    }

    mFile.Write(gLogIntro.data(), gLogIntro.length());
}

LoggerBackendHTML::~LoggerBackendHTML()
{
    const static std::string gLogOutro = R"(
            </tbody>
        </table>
        <script src='../nfEngine/Data/tablefilter.js'></script>
    </body>
</html>
)";

    mFile.Write(gLogOutro.data(), gLogOutro.length());
}

void LoggerBackendHTML::Log(LogType type, const char* srcFile, int line, const char* str,
                            double timeElapsed)
{
    // begin row tag with style + begin column tag
    const char* str0 = nullptr;
    switch (type)
    {
    case LogType::Info:
        str0 = "<tr class=\"info\">";
        break;
    case LogType::OK:
        str0 = "<tr class=\"ok\">";
        break;
    case LogType::Warning:
        str0 = "<tr class=\"warn\">";
        break;
    case LogType::Error:
        str0 = "<tr class=\"err\">";
        break;
    case LogType::Fatal:
        str0 = "<tr class=\"fatal\">";
        break;
    default:
        str0 = "<tr class=\"info\">";
        break;
    }

    // close column tag + begin column tag with right justification
    const char* str1 = "</td><td class=\"srcFile\" nowrap='nowrap'>";

    // close column tag + close row column
    const char* str2 = "</td></tr>\n";

    // string format for source file and line number information
    const char* format = R"(%s<td class="time">[%.4f]</td><td class="message">%s%s<a href="file:///%s">%s</a>:%i%s)";


    size_t pathOffset = Logger::GetInstance()->GetPathPrefixLen();
    int len = snprintf(mBuffer.data(), mBuffer.size(), format,
                       str0, timeElapsed, str, str1, srcFile, srcFile + pathOffset, line, str2);
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
                 str0, timeElapsed, str, str1, srcFile, srcFile + pathOffset, line, str2);
    }

    mFile.Write(mBuffer.data(), outputStrSize);
}

} // namespace Common
} // namespace NFE
