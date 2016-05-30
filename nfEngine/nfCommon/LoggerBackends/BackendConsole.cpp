/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of console (standard output) logger backend
 */

#include "../PCH.hpp"
#include "BackendConsole.hpp"
#include "../Logger.hpp"
#include "../Console.hpp"

namespace NFE {
namespace Common {

// Register Console backend
bool gLoggerBackendConsoleRegistered = Logger::RegisterBackend("Console", new LoggerBackendConsole);

namespace {

ConsoleColor LogTypeToColor(LogType logType)
{
    switch (logType)
    {
    case LogType::Debug:
        return ConsoleColor::Cyan;
    case LogType::OK:
        return ConsoleColor::Green;
    case LogType::Warning:
        return ConsoleColor::Yellow;
    case LogType::Error:
        return ConsoleColor::Red;
    case LogType::Fatal:
        return ConsoleColor::Red | ConsoleColor::Intense;
    case LogType::Info:
    default:
        return ConsoleColor::White;
    }
}

} // namespace

LoggerBackendConsole::LoggerBackendConsole()
{
    mIsEnabled = true;
}

void LoggerBackendConsole::Log(LogType type, const char* srcFile, int line, const char* str,
                               double timeElapsed)
{
    PrintColored(LogTypeToColor(type),
                 "%.4f [%-7s] %s:%i: %s\n",
                 timeElapsed,
                 Logger::LogTypeToString(type),
                 srcFile, line,
                 str);
}

} // namespace Common
} // namespace NFE
