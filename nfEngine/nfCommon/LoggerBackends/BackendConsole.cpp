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

void LoggerBackendConsole::Log(LogType type, const char* function, const char* srcFile,
                               int line, const char* str, double timeElapsed)
{
    (void)function;

    printf("[%s] %f %s:%i %s\n", Logger::LogTypeToString(type), timeElapsed, srcFile, line, str);
}

} // namespace Common
} // namespace NFE
