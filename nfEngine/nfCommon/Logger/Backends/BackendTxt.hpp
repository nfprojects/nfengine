/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Declaration of raw txt logger backend
 */

#pragma once

#include "../Logger.hpp"
#include "FileSystem/File.hpp"


namespace NFE {
namespace Common {

/**
 * Raw txt logger backend implementation.
 */
class NFCOMMON_API LoggerBackendTxt final : public LoggerBackend
{
    File mFile;
    std::vector<char> mBuffer;

public:
    LoggerBackendTxt();

    void Reset() override;
    void Log(LogType type, const char* srcFile, int line, const char* str,
             double timeElapsed) override;
};

} // namespace Common
} // namespace NFE
