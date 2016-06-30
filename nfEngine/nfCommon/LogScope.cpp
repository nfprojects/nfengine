/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of LogScope class
 */

#include "PCH.hpp"
#include "LogScope.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Common {

LogScope::LogScope()
    : LogScope("Scope")
{
}

LogScope::LogScope(const char* name)
    : mScopeName(name)
{
    LOG_INFO("Entering scope %s.", mScopeName);
}

LogScope::~LogScope()
{
    LOG_INFO("Exiting scope %s.", mScopeName);
}

} // namespace Common
} // namespace NFE
