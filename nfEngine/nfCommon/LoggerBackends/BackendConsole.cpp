/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of console (standard output) logger backend
 */

#include "../PCH.hpp"
#include "BackendConsole.hpp"
#include "../Console.hpp"

namespace NFE {
namespace Common {

namespace {

ConsoleColor LogTypeToColor(LogType logType)
{
    switch (logType)
    {
    case LogType::Debug:
        return ConsoleColor::Cyan;
    case LogType::Info:
        return ConsoleColor::White;
    case LogType::OK:
        return ConsoleColor::Green;
    case LogType::Warning:
        return ConsoleColor::Yellow;
    case LogType::Error:
        return ConsoleColor::Red;
    case LogType::Fatal:
        return ConsoleColor::Red | ConsoleColor::Intense;
    }

    return ConsoleColor::White;
}

} // namespace

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
