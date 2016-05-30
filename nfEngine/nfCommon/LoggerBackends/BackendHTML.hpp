/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of HTML logger backend
 */

#pragma once

#include "../Logger.hpp"
#include "../File.hpp"

namespace NFE {
namespace Common {

/**
 * HTML logger backend implementation.
 */
class NFCOMMON_API LoggerBackendHTML : public LoggerBackend
{
    File mFile;
    std::vector<char> mBuffer;

public:
    LoggerBackendHTML();
    ~LoggerBackendHTML();

    void Reset();
    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed);
};

} // namespace Common
} // namespace NFE
