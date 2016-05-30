/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of console (standard output) logger backend
 */

#pragma once

#include "../Logger.hpp"

namespace NFE {
namespace Common {

/**
 * Console logger backend implementation.
 */
class NFCOMMON_API LoggerBackendConsole : public LoggerBackend
{
public:
    LoggerBackendConsole();
    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed);
};

} // namespace Common
} // namespace NFE
