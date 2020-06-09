/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of HTML logger backend
 */

#pragma once

#include "../LoggerBackend.hpp"
#include "../../FileSystem/File.hpp"
#include "../../Containers/DynArray.hpp"


namespace NFE {
namespace Common {

/**
 * HTML logger backend implementation.
 */
class NFCOMMON_API LoggerBackendHTML final : public ILoggerBackend
{
    File mFile;
    DynArray<char> mBuffer;

public:
    LoggerBackendHTML();
    ~LoggerBackendHTML();

    bool Init() override;
    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed) override;
};

} // namespace Common
} // namespace NFE
