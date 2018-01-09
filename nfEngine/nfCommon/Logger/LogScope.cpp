/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Definition of LogScope class
 */

#include "PCH.hpp"
#include "LogScope.hpp"
#include "Logger/Logger.hpp"

namespace NFE {
namespace Common {

LogScope::LogScope()
    : LogScope("Scope")
{
}

LogScope::LogScope(const char* name)
    : mScopeName(name)
{
    NFE_LOG_INFO("Entering scope %s.", mScopeName);
}

LogScope::LogScope(const char* name, const char* file, int line)
    : mScopeName(name)
{
    NFE_LOG_INFO("Entering scope %s in %s@%i.", mScopeName, file, line);
}

LogScope::~LogScope()
{
    NFE_LOG_INFO("Exiting scope %s.", mScopeName);
}

} // namespace Common
} // namespace NFE
