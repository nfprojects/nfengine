/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of HTML logger backend
 */

#include "../PCH.hpp"
#include "BackendHTML.hpp"
#include "../Logger.hpp"

 // TODO Remove when std::make_unique is available in gcc & clang
#if defined(__LINUX__) | defined(__linux__)
#include "../Linux/MakeUniqueTemp.hpp"
#endif

#define NFE_MAX_LOG_MESSAGE_LENGTH 1024

/// snprintf was not implemented until Visual Studio 2015
#if defined(WIN32) && defined(_MSC_VER)
#if _MSC_VER <= 1800
#define snprintf sprintf_s
#endif
#endif

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

<style type="text/css">
    body { background: #000000; color: #E0E0E0; margin-right: 6px; margin-left: 6px;
           font-size: 13px; font-family: "Lucida Console", monospace, sans-serif; }
    a { text-decoration: none; }
    a:link { color: #b4c8d2; }
    a:active { color: #ff9900; }
    a:visited { color: #b4c8d2; }
    a:hover { color: #ff9900; }
    h1 { text-align: center; }
    h2 { color: #ffffff; }
        .dbg, .info, .ok, .warn, .err, .fatal,
        .dbg { background-color: #000000; color: #333399; padding: 0px; }
        .dbg:hover { background-color: #101030; color: #4040ff; padding: 0px; }
        .info { background-color: #000000; color: #d4d8d2; padding: 0px; }
        .info:hover { background-color: #202020; color: #ffffff; padding: 0px; }
        .ok { background-color: #000000; color: #339933; padding: 0px; }
        .ok:hover { background-color: #103010; color: #40ff40; padding: 0px; }
        .warn { background-color: #281c10; color: #F2B13A; padding: 0px; }
        .warn:hover { background-color: #303030; color: #ff7b00; padding: 0px; }
        .err { background-color: #301010; color: #ff5e5e; padding: 0px; }
        .err:hover { background-color: #602020; color: #ffffff; padding: 0px; }
        .fatal { background-color: #ff0000; color: #000000; padding: 0px; }
        .fatal:hover { background-color: #303030; color: #ff0000; padding: 0px; }
</style>
</head>

<body>
<h3>nfEngine - log file</h3>
<table width="100%%" cellspacing="0" cellpadding="0" bordercolor="080c10" border="0" rules="none">
<tr class="table_info">
    <td nowrap="nowrap">[<i>Seconds elapsed</i>] <i>Message...</i></td>
    <td nowrap="nowrap" align="right"><i>SourceFile</i>:<i>LineOfCode</i></td>
</tr>
</table><br>

<table width="100%%" cellspacing="0" cellpadding="0" bordercolor="080c10" border="0" rules="none">
)";

    const std::string logFileName = "log.html";
    const size_t initialBufferSize = 1024;
    mBuffer.resize(initialBufferSize);

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
</table>
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
        str0 = "<tr class=\"info\"><td>";
        break;
    case LogType::OK:
        str0 = "<tr class=\"ok\"><td>";
        break;
    case LogType::Warning:
        str0 = "<tr class=\"warn\"><td>";
        break;
    case LogType::Error:
        str0 = "<tr class=\"err\"><td>";
        break;
    case LogType::Fatal:
        str0 = "<tr class=\"fatal\"><td>";
        break;
    default:
        str0 = "<tr class=\"info\"><td>";
        break;
    }

    // close column tag + begin column tag with right justification
    const char* str1 = "</td><td nowrap=\"nowrap\" align=\"right\">";

    // close column tag + close row column
    const char* str2 = "</td></tr>\n";

    // string format for source file and line number information
    const char* format = R"(%s[%.4f] %s%s<a href="file:///%s">%s</a>:%i%s)";


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
