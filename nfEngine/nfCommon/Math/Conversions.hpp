/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of functions for types conversions.
 */

#pragma once

#include "../nfCommon.hpp"
#include "Math.hpp"


namespace NFE {
namespace Math {

using HalfFloat = uint16;

NFCOMMON_API HalfFloat ConvertFloatToHalfFloat(float value);
NFCOMMON_API float ConvertHalfFloatToFloat(HalfFloat value);
NFCOMMON_API uint8 ConvertFloatToUint8(float x);
NFCOMMON_API uint8 ConvertFloatToNormUint8(float x);
NFCOMMON_API int8 ConvertFloatToInt8(float x);
NFCOMMON_API int8 ConvertFloatToNormInt8(float x);
NFCOMMON_API uint16 ConvertFloatToUint16(float x);
NFCOMMON_API uint16 ConvertFloatToNormUint16(float x);
NFCOMMON_API int16 ConvertFloatToInt16(float x);
NFCOMMON_API int16 ConvertFloatToNormInt16(float x);
NFCOMMON_API uint32 ConvertFloatToUint32(float x);
NFCOMMON_API int32 ConvertFloatToInt32(float x);

} // namespace Math
} // namespace NFE
