/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of ILoggerBackend class
 */

#include "PCH.hpp"
#include "LoggerBackend.hpp"


namespace NFE {
namespace Common {


ILoggerBackend::ILoggerBackend()
    : mIsEnabled(false)
{
}

ILoggerBackend::~ILoggerBackend() = default;

bool ILoggerBackend::Init()
{
    return true;
}


} // namespace Common
} // namespace NFE
