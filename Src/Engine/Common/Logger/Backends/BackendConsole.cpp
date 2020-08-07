/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of console (standard output) logger backend
 */

#include "PCH.hpp"
#include "../LoggerImpl.hpp"
#include "BackendConsole.hpp"
#include "System/Console.hpp"
#include "../../Containers/StringView.hpp"
#include "../../System/Thread.hpp"


namespace NFE {
namespace Common {

// Register Console backend
bool gLoggerBackendConsoleRegistered = Logger::RegisterBackend(StringView("Console"), MakeUniquePtr<LoggerBackendConsole>());

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
    const uint32 currentThreadID = Thread::GetCurrentThreadID();

    PrintColored(LogTypeToColor(type),
                 "%.4f {%u} [%-7s] %s:%i: %s\n",
                 timeElapsed,
                 currentThreadID,
                 Logger::LogTypeToString(type),
                 srcFile, line,
                 str);
}

} // namespace Common
} // namespace NFE
