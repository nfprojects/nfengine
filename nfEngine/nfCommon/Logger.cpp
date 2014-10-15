#include "stdafx.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Common {

Logger::Logger()
{
}

Logger::~Logger()
{
    CloseAll();
}

Logger* InitStaticLogger()
{
    CreateDirectoryA("Logs/", 0);
    Logger* pLogger = new Logger;

    if (pLogger)
    {
        pLogger->OpenFile("Logs/nfEngine_full_log.html", Html);
        pLogger->OpenFile("Logs/raw.txt", Raw);
    }

    return pLogger;
}

Logger* Logger::GetInstance()
{
    static std::unique_ptr<Logger> s_Logger(InitStaticLogger());
    return s_Logger.get();
}

int Logger::OpenFile(const char* pFile, LoggerOutputType outputType)
{
    LoggerOutput* pLoggerOutput = new LoggerOutput;

    pLoggerOutput->file.open(pFile, std::ios::out);

    if (!pLoggerOutput->file.good())
        return 1;

    time_t rawtime;
    tm timeinfo;

    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);

    // Nov 24 2011, 23:21:01
    char pDateTimeStr[32];
    strftime(pDateTimeStr, 128, "%b %d %Y, %X", &timeinfo);

    switch (outputType)
    {
        case Html:
            pLoggerOutput->file <<
                                "<?xml version=\"1.0\" encoding=\"utf-8\"?> <!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">"
                                "<html><head> <title>nfEngine log</title> <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>"
                                "<style type=\"text/css\"> body { background: #404040; color: #E0E0E0; margin-right: 6px; margin-left: 6px; font-size: 13px; font-family: Lucida Console, sans-serif, sans; } a { text-decoration: none; } a:link { color: #b4c8d2; } a:active { color: #ff9900; } a:visited { color: #b4c8d2; } a:hover { color: #ff9900; } h1 { text-align: center; } h2 { color: #ffffff; }"


                                ".source, .table_info, .message, .success, .warning, .error, .fatal, "
                                ".source { background-color: #101010; color: #212221; padding: 0px; } "
                                ".source:hover { background-color: #202430; color: #ffffff; padding: 0px; } "

                                ".table_info { background-color: #101010; color: #ffffff; padding: 40px; } "
                                ".table_info:hover { background-color: #202430; color: #ffffff; padding: 40px; } "

                                ".message { background-color: #101010; color: #d4d8d2; padding: 0px; } "
                                ".message:hover { background-color: #202430; color: #ffffff; padding: 0px; } "

                                ".success { background-color: #101010; color: #339933; padding: 0px; } "
                                ".success:hover { background-color: #202430; color: #40ff40; padding: 0px; } "

                                ".warning { background-color: #281c10; color: #F2B13A; padding: 0px; } "
                                ".warning:hover { background-color: #202430; color: #FF7B00; padding: 0px; } "

                                ".error { background-color: #380f10; color: #FF5E5E; padding: 0px; } "
                                ".error:hover { background-color: #202430; color: #ff0000; padding: 0px; } "

                                ".fatal { background-color: #ff0000; color: #000000; padding: 0px; } "
                                ".fatal:hover { background-color: #202430; color: #ff0000; padding: 0px; } "

                                "</style></head><body>\n"
                                "<h3>nfEngine v0.1 - log file</h2>"

                                "<h3><div padding:3px>Build: " __DATE__ ", " __TIME__ ", Run:  " << pDateTimeStr << "</h3>"

                                "\n<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\" bordercolor=\"080c10\" border=\"0\" rules=\"none\" >\n"
                                "\n<tr class=\"table_info\">\n<td nowrap=\"nowrap\">[<i>Seconds elapsed</i>] {<i>ThreadID</i>} <i>Message...</i></td>\n<td nowrap=\"nowrap\" align=\"right\"><i>Source file</i> : <i>Line of code</i> @ <i>Calling function</i></td>\n</tr>"
                                "\n</table><br>\n"

                                "\n<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\" bordercolor=\"080c10\" border=\"0\" rules=\"none\" >\n";
            break;

        case Raw:
            pLoggerOutput->file << "nfEngine v0.1 - log file\n\n";
            break;

        default:
            return 1;
    };

    pLoggerOutput->outputType = outputType;
    mOutputs[std::string(pFile)] = pLoggerOutput;

    mTimer.Start();
    return 0;
}

void Logger::CloseAll()
{
    for (auto& loggerOut : mOutputs)
    {
        if (loggerOut.second->outputType == Html)
        {
            loggerOut.second->file << "</table>\n</body>\n";
        }

        loggerOut.second->file.close();
        delete loggerOut.second;
    }

    mOutputs.clear();
}

#define MAX_LOG_MESSAGE_LENGTH 1024

void Logger::Log(LogType type, const char* pFunction, const char* pSource, int line,
                 const char* pStr, ...)
{
    double t = mTimer.Stop();

    char buffer[MAX_LOG_MESSAGE_LENGTH];
    va_list args;
    va_start(args, pStr);
    vsprintf_s(buffer, MAX_LOG_MESSAGE_LENGTH, pStr, args);

    // lock I/O
    std::unique_lock<std::mutex> lock(mMutex);

    //write to console
    OutputDebugStringA("NFEngine: ");
    if (type == LogType::Error) OutputDebugStringA("[ERROR] ");
    if (type == LogType::Fatal) OutputDebugStringA("[FATAL ERROR] ");
    if (type == LogType::Warning) OutputDebugStringA("[WARNING] ");
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");

    for (auto& loggerOut : mOutputs)
    {
        if (loggerOut.second->outputType == Html)
        {
            switch (type)
            {
                case LogType::Info:
                    loggerOut.second->file << "\n<tr class=\"message\">\n<td nowrap=\"nowrap\">";
                    break;

                case LogType::Success:
                    loggerOut.second->file << "\n<tr class=\"success\">\n<td nowrap=\"nowrap\">";
                    break;

                case LogType::Warning:
                    loggerOut.second->file << "\n<tr class=\"warning\">\n<td nowrap=\"nowrap\">";
                    break;

                case LogType::Error:
                    loggerOut.second->file << "\n<tr class=\"error\">\n<td nowrap=\"nowrap\">";
                    break;

                case LogType::Fatal:
                    loggerOut.second->file << "\n<tr class=\"fatal\">\n<td nowrap=\"nowrap\">";
                    break;

                default:
                    loggerOut.second->file << "\n<tr class=\"message\">\n<td nowrap=\"nowrap\">";
                    break;
            }

            loggerOut.second->file << "[" << std::fixed << std::setprecision(4) << t
                                   << "] {" << std::this_thread::get_id() << "} "
                                   << buffer << "</td>\n<td nowrap=\"nowrap\" align=\"right\">"
                                   << pSource << " : " << line << " @ " << pFunction << "</td>\n</tr>";
        }

        if (loggerOut.second->outputType == Raw)
            loggerOut.second->file << "[" << t << "] " << buffer << "\n";

        loggerOut.second->file.flush();
    }
}

} // namespace Common
} // namespace NFE
