/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of Windows debugger logger backend
 */

#pragma once

#include "../../LoggerBackend.hpp"
#include "../../../Containers/DynArray.hpp"
#include "../../../Containers/String.hpp"
#include "../../../System/Win/Common.hpp"

namespace NFE {
namespace Common {

/**
 * Implementation of Windows debugger logger backend.
 */
class NFCOMMON_API LoggerBackendWinDebugger final : public LoggerBackend
{
    DynArray<char> mBuffer;
    String mDebugString;
    Utf16String mWideDebugString;

public:
    LoggerBackendWinDebugger();
    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed) override;
};

} // namespace Common
} // namespace NFE
