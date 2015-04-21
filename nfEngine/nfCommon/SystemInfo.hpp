/**
 * @file   SystemInfo.hpp
 * @author mkulagowski (mkkulagowski@gmail.com)
 * @brief  System information API declarations.
 */

#pragma once

#include "nfCommon.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Common {

/**
 * Logs CPU information, cache line size etc.
 */
NFCOMMON_API void ReadHardwareInfo();

} // namespace Common
} // namespace NFE