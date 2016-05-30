/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Declaration of raw txt logger backend
 */

#pragma once

#include "../Logger.hpp"
#include "../File.hpp"

namespace NFE {
namespace Common {

/**
 * Raw txt logger backend implementation.
 */
class NFCOMMON_API LoggerBackendTxt : public LoggerBackend
{
    File mFile;
    std::vector<char> mBuffer;

public:
    LoggerBackendTxt();

    void Reset();
    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed);
};

} // namespace Common
} // namespace NFE
