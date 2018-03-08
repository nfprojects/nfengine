/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of LoggerBackend class
 */

#include "PCH.hpp"
#include "LoggerBackend.hpp"


namespace NFE {
namespace Common {


LoggerBackend::LoggerBackend()
    : mIsEnabled(true)
{
}

LoggerBackend::~LoggerBackend() = default;

void LoggerBackend::Reset()
{
}


} // namespace Common
} // namespace NFE
