/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of console (standard output) logger backend
 */

#pragma once

#include "../LoggerBackend.hpp"


namespace NFE {
namespace Common {

/**
 * Console logger backend implementation.
 */
class NFCOMMON_API LoggerBackendConsole final : public ILoggerBackend
{
public:
    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed) override;
};

} // namespace Common
} // namespace NFE
