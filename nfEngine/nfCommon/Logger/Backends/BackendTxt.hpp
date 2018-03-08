/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Declaration of raw txt logger backend
 */

#pragma once

#include "../LoggerBackend.hpp"
#include "../../FileSystem/File.hpp"
#include "../../Containers/DynArray.hpp"


namespace NFE {
namespace Common {

/**
 * Raw txt logger backend implementation.
 */
class NFCOMMON_API LoggerBackendTxt final : public LoggerBackend
{
    File mFile;
    DynArray<char> mBuffer;

public:
    LoggerBackendTxt();

    void Reset() override;
    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed) override;
};

} // namespace Common
} // namespace NFE
