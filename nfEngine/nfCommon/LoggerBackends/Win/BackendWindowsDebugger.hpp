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
class NFCOMMON_API LoggerBackendWinDebugger : public LoggerBackend
{
    std::vector<char> mBuffer;

    std::string mDebugString;
    std::wstring mWideDebugString;

public:
    LoggerBackendWinDebugger();

    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed);
};

} // namespace Common
} // namespace NFE
