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
#if _MSC_VER <= 1800
#define snprintf sprintf_s
#endif
#endif

namespace NFE {
namespace Common {

LoggerBackendHTML::LoggerBackendHTML()
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
    </head>
    <body>
        <h3>nfEngine - log file</h3>
        <table id='log'>
        <col width='7%' valign='top'>
        <col width='74%' valign='top'>
        <col width='19%' valign='middle'>
        <thead>
            <tr>
                <th>Seconds elapsed</th>
                <th>Message ...</th>
                <th>SourceFile : LineOfCode</th>
            </tr>
        </thead>
        <tbody>
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
            </tbody>
        </table>
        <script src='../nfEngineDeps/TableFilter/tablefilter.js'></script>
        <script>
            var filtersConfig = {
                    base_path: '../nfEngineDeps/TableFilter',
                    auto_filter: true,
                    auto_filter_delay: 500, // milliseconds
                    filters_row_index: 1,
                    status_nar:true,
                    btn_reset: true,
                    msg_filter: 'Filtering...',
                    no_results_message: true,
                    highlight_keywords: true,
                    watermark: 'Search...'
                };
            var tf = new TableFilter('log', filtersConfig);
            tf.init();
        </script>
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
    const char* str1 = "</td><td nowrap='nowrap'>";

    // close column tag + close row column
    const char* str2 = "</td></tr>\n";

    // string format for source file and line number information
    const char* format = R"(%s<td>[%.4f]</td><td>%s%s<a href="file:///%s">%s</a>:%i%s)";


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
