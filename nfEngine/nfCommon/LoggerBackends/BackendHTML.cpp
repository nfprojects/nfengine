/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of HTML logger backend
 */

#include "../PCH.hpp"
#include "BackendHTML.hpp"

#define NFE_MAX_LOG_MESSAGE_LENGTH 1024

namespace NFE {
namespace Common {

namespace {

std::string gLogIntro = R"(
<?xml version="1.0" encoding="utf-8"?> <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html><head> <title>nfEngine log</title> <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
<style type="text/css">
    body { background: #000000; color: #E0E0E0; margin-right: 6px; margin-left: 6px; font-size: 13px; font-family: Lucida Console, sans-serif, sans; }
    a { text-decoration: none; }
    a:link { color: #b4c8d2; }
    a:active { color: #ff9900; }
    a:visited { color: #b4c8d2; }
    a:hover { color: #ff9900; }
    h1 { text-align: center; }
    h2 { color: #ffffff; }
        .source, .table_info, .message, .success, .warning, .error, .fatal,
        .source { background-color: #101010; color: #212221; padding: 0px; }
        .source:hover { background-color: #202430; color: #ffffff; padding: 0px; }
        .table_info { background-color: #101010; color: #ffffff; padding: 40px; }
        .table_info:hover { background-color: #202430; color: #ffffff; padding: 40px; }
        .message { background-color: #101010; color: #d4d8d2; padding: 0px; }
        .message:hover { background-color: #202430; color: #ffffff; padding: 0px; }
        .success { background-color: #101010; color: #339933; padding: 0px; }
        .success:hover { background-color: #202430; color: #40ff40; padding: 0px; }
        .warning { background-color: #281c10; color: #F2B13A; padding: 0px; }
        .warning:hover { background-color: #202430; color: #FF7B00; padding: 0px; }
        .error { background-color: #380f10; color: #FF5E5E; padding: 0px; }
        .error:hover { background-color: #202430; color: #ff0000; padding: 0px; }
        .fatal { background-color: #ff0000; color: #000000; padding: 0px; }
        .fatal:hover { background-color: #202430; color: #ff0000; padding: 0px; }
</style></head><body>

<h3>nfEngine - log file</h2>
<h3><div padding:3px>Build: TODO, Run: TODO "</h3>

<table width="100%%" cellspacing="0" cellpadding="0" bordercolor="080c10" border="0" rules="none">
<tr class="table_info">
    <td nowrap="nowrap">[<i>Seconds elapsed</i>] {<i>ThreadID</i>} <i>Message...</i></td>
    <td nowrap="nowrap" align="right"><i>Source file</i> : <i>Line of code</i> @ <i>Calling function</i></td>
</tr>
</table><br>

<table width="100%%" cellspacing="0" cellpadding="0" bordercolor="080c10" border="0" rules="none">

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

void LoggerBackendHTML::Log(LogType type, const char* function, const char* srcFile,
                            int line, const char* str, double timeElapsed)
{
    const char* str0 = nullptr;
    switch (type)
    {
    case LogType::Info:
        str0 = "<tr class=\"message\"><td nowrap=\"nowrap\">";
        break;
    case LogType::Success:
        str0 = "<tr class=\"success\"><td nowrap=\"nowrap\">";
        break;
    case LogType::Warning:
        str0 = "<tr class=\"warning\"><td nowrap=\"nowrap\">";
        break;
    case LogType::Error:
        str0 = "<tr class=\"error\"><td nowrap=\"nowrap\">";
        break;
    case LogType::Fatal:
        str0 = "<tr class=\"fatal\"><td nowrap=\"nowrap\">";
        break;
    default:
        str0 = "<tr class=\"message\"><td nowrap=\"nowrap\">";
        break;
    }

    char str1[128];
    sprintf(str1, "[%.4f] ", timeElapsed);

    const char* str2 = "</td><td align=\"right\">";
    const char* str3 = " : ";

    char str4[128];
    sprintf(str4, "%i", line);

    const char* str5 = " @ ";
    const char* str6 = "</td></tr>\n";

    mFile.Write(str0, strlen(str0));
    mFile.Write(str, strlen(str));
    mFile.Write(str2, strlen(str2));
    mFile.Write(srcFile, strlen(srcFile));
    mFile.Write(str3, strlen(str3));
    mFile.Write(str4, strlen(str4));
    mFile.Write(str5, strlen(str5));
    mFile.Write(function, strlen(function));
    mFile.Write(str6, strlen(str6));
}

} // namespace Common
} // namespace NFE
