/**
 * @file   SystemInfo.hpp
 * @author mkulagowski (mkkulagowski@gmail.com)
 * @brief  System information API declarations.
 */

#pragma once

#include "nfCommon.hpp"
#include "Logger.hpp"

namespace {
    const int EXTENDED_ID_MAX_VALUE = 0x80000000;
    const int CPU_BRAND_STRING_LENGTH = 0x40;
    const int CPU_BRAND_STRING_1 = 0x80000002;
    const int CPU_BRAND_STRING_2 = 0x80000003;
    const int CPU_BRAND_STRING_3 = 0x80000004;
}

namespace NFE {
namespace Common {

/**
 * Logs CPU information, cache line size etc.
 */
NFCOMMON_API void ReadHardwareInfo();

} // namespace Common
} // namespace NFE