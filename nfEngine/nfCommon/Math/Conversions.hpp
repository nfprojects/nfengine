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

NFCOMMON_API HalfFloat ToHalfFloat(float value);
NFCOMMON_API float ToFloat(HalfFloat value);
NFCOMMON_API uint8 ToUint8(float x);
NFCOMMON_API uint8 ToNormUint8(float x);
NFCOMMON_API int8 ToInt8(float x);
NFCOMMON_API int8 ToNormInt8(float x);
NFCOMMON_API uint16 ToUint16(float x);
NFCOMMON_API uint16 ToNormUint16(float x);
NFCOMMON_API int16 ToInt16(float x);
NFCOMMON_API int16 ToNormInt16(float x);
NFCOMMON_API uint32 ToUint32(float x);
NFCOMMON_API int32 ToInt32(float x);

} // namespace Math
} // namespace NFE
