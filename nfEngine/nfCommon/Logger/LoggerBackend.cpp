/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of ILoggerBackend interface
 */

#include "PCH.hpp"
#include "LoggerBackend.hpp"


namespace NFE {
namespace Common {


ILoggerBackend::ILoggerBackend()
    : mIsEnabled(true)
{
}

ILoggerBackend::~ILoggerBackend() = default;

void ILoggerBackend::Reset()
{
}


} // namespace Common
} // namespace NFE
