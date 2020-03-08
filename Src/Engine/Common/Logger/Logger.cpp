/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of Logger class
 */

#include "PCH.hpp"
#include "LoggerImpl.hpp"



namespace NFE {
namespace Common {

ILogger::ILogger() = default;
ILogger::~ILogger() = default;

ILogger* ILogger::GetInstance()
{
    return Logger::GetInstance();
}

} // namespace Common
} // namespace NFE
