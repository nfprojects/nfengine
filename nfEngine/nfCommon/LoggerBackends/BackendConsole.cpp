/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of console (standard output) logger backend
 */

#include "../PCH.hpp"
#include "BackendConsole.hpp"

#define NFE_MAX_LOG_MESSAGE_LENGTH 1024

namespace NFE {
namespace Common {

const char* LogTypeToString(LogType type)
{
    switch (type)
    {
    case LogType::Info:
        return "INFO";
    case LogType::Success:
        return "SUCCESS";
    case LogType::Warning:
        return "WARNING";
    case LogType::Error:
        return "ERROR";
    case LogType::Fatal:
        return "FATAL";
    }

    return "UKNOWN";
}

void LoggerBackendConsole::Log(LogType type, const char* function, const char* srcFile,
                               int line, const char* str, double timeElapsed)
{
    (void)function;
    printf("[%s] %f %s:%i %s\n", LogTypeToString(type), timeElapsed, srcFile, line, str);
}

} // namespace Common
} // namespace NFE
