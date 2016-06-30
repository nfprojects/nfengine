/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  LogScope class declarations.
 */

#pragma once

#include "Logger.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API LogScope
{
private:
    const char* mScopeName;

public:
    NFE_INLINE LogScope(const char* name)
        : mScopeName(name)
    {
        LOG_INFO("Entering scope %s.", mScopeName);
    };

    NFE_INLINE ~LogScope()
    {
        LOG_INFO("Exiting scope %s.", mScopeName);
    };
};

} // namespace Common
} // namespace NFE