/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of HTML logger backend
 */

#include "../PCH.hpp"
#include "BackendHTML.hpp"

#define NFE_MAX_LOG_MESSAGE_LENGTH 1024

/// snprintf was not implemented until Visual Studio 2015
#if defined(WIN32) && defined(_MSC_VER)
#if _MSC_VER <= 1700
#define snprintf sprintf_s
#endif
#endif

namespace NFE {
namespace Common {

namespace {

std::string gLogIntro = R"(<?xml version="1.0" encoding="utf-8"?>
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
        .source, .table_info, .message, .success, .warning, .error, .fatal,
        .source { background-color: #000000; color: #212221; padding: 0px; }
        .source:hover { background-color: #303030; color: #ffffff; padding: 0px; }
        .table_info { background-color: #000000; color: #ffffff; padding: 40px; }
        .table_info:hover { background-color: #303030; color: #ffffff; padding: 40px; }
        .message { background-color: #000000; color: #d4d8d2; padding: 0px; }
        .message:hover { background-color: #303030; color: #ffffff; padding: 0px; }
        .success { background-color: #000000; color: #339933; padding: 0px; }
        .success:hover { background-color: #303030; color: #40ff40; padding: 0px; }
        .warning { background-color: #281c10; color: #F2B13A; padding: 0px; }
        .warning:hover { background-color: #303030; color: #ff7b00; padding: 0px; }
        .error { background-color: #301010; color: #ff5e5e; padding: 0px; }
        .error:hover { background-color: #602020; color: #ffffff; padding: 0px; }
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

std::string gLogOutro = R"(
</table>
</body>
</html>
)";

} // namespace

LoggerBackendHTML::LoggerBackendHTML()
{
    if (!mFile.Open("Logs/log.html", AccessMode::Write, true))
    {
        // this will be handled by other logger
        LOG_ERROR("Failed to open output file");
        return;
    }

    mFile.Write(gLogIntro.data(), gLogIntro.length());
}

LoggerBackendHTML::~LoggerBackendHTML()
{
    mFile.Write(gLogOutro.data(), gLogOutro.length());
}

void LoggerBackendHTML::Log(LogType type, const char* srcFile, int line, const char* str,
                            double timeElapsed)
{
    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];


    const char* str0 = nullptr;
    const char* str1 = "</td><td nowrap=\"nowrap\" align=\"right\">";
    const char* str2 = "</td></tr>\n";

    switch (type)
    {
    case LogType::Info:
        str0 = "<tr class=\"message\"><td>";
        break;
    case LogType::Success:
        str0 = "<tr class=\"success\"><td>";
        break;
    case LogType::Warning:
        str0 = "<tr class=\"warning\"><td>";
        break;
    case LogType::Error:
        str0 = "<tr class=\"error\"><td>";
        break;
    case LogType::Fatal:
        str0 = "<tr class=\"fatal\"><td>";
        break;
    default:
        str0 = "<tr class=\"message\"><td>";
        break;
    }

    int len = snprintf(buffer, BUFFER_SIZE, "%s[%.4f] %s%s%s:%i%s",
                       str0, timeElapsed, str, str1, srcFile, line, str2);

    if (len > 0)
        mFile.Write(buffer, static_cast<size_t>(len));
}

} // namespace Common
} // namespace NFE
