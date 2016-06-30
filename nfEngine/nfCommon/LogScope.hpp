/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  LogScope class declarations.
 */

#pragma once

#include "nfCommon.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API LogScope
{
private:
    const char* mScopeName;

public:
    LogScope();
    LogScope(const char* name);
    ~LogScope();
};

} // namespace Common
} // namespace NFE