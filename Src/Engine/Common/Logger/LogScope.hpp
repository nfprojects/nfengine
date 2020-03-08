/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  LogScope class declarations.
 */

#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API LogScope
{
private:
    const char* mScopeName;

public:
    LogScope();
    LogScope(const char* name);
    LogScope(const char* name, const char* file, int line);
    ~LogScope();
};

// Macro for easy calling LogScope's constructor with __FILE__ and __LINE__.
#define LOG_SCOPE(scopeName) LogScope scopeName(#scopeName, __FILE__, __LINE__)

} // namespace Common
} // namespace NFE