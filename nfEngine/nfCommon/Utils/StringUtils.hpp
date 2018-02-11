/**
 * @file    Witek902
 * @brief   String utilities declarations.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Containers/String.hpp"


namespace NFE {
namespace Common {


NFCOMMON_API String ToString(uint32 value);
NFCOMMON_API String ToString(uint64 value);
NFCOMMON_API String ToString(int32 value);
NFCOMMON_API String ToString(int64 value);
NFCOMMON_API String ToString(float value);
NFCOMMON_API String ToString(double value);


} // namespace Common
} // namespace NFE
