/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of console (standard output) logger backend
 */

#include "../PCH.hpp"
#include "BackendConsole.hpp"
#include "../Logger.hpp"
#include "../Console.hpp"

 // TODO Remove when std::make_unique is available in gcc & clang
#if defined(__LINUX__) | defined(__linux__)
#include "../Linux/MakeUniqueTemp.hpp"
#endif

namespace NFE {
namespace Common {

// Register Console backend
bool gLoggerBackendConsoleRegistered = Logger::RegisterBackend("Console", std::make_unique<LoggerBackendConsole>());

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
