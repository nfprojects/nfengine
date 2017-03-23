/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Windows debugger logger backend
 */

#pragma once

#include "../../Logger.hpp"

namespace NFE {
namespace Common {

/**
 * Implementation of Windows debugger logger backend.
 */
class NFCOMMON_API LoggerBackendWinDebugger final : public LoggerBackend
{
    std::vector<char> mBuffer;

    String mDebugString;
    std::wstring mWideDebugString;

public:
    LoggerBackendWinDebugger();
    void Log(LogType type, const char* srcFile, int line, const char* str,
             double timeElapsed) override;
};

} // namespace Common
} // namespace NFE
