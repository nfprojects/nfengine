/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of functions for types conversions.
 */

#include "../PCH.hpp"
#include "Conversions.hpp"

namespace NFE {
namespace Math {


namespace {

const int32 HALF_SHIFT = 13;
const int32 HALF_SHIFT_SIGN = 16;
const int32 FLOAT_POS_INF = 0x7F800000; // flt32 infinity
const int32 HALF_MAX_NORM_AS_32 = 0x477FE000; // max flt16 normal as a flt32
const int32 HALF_MIN_NORM_AS_32 = 0x38800000; // min flt16 normal as a flt32
const int32 FLOAT_SIGN_BIT = 0x80000000; // flt32 sign bit
const int32 HALF_MIN_NAN_AS_32 = ((FLOAT_POS_INF >> HALF_SHIFT) + 1) << HALF_SHIFT; // minimum flt16 nan as a flt32
const int32 HALF_MAX_NORM = HALF_MAX_NORM_AS_32 >> HALF_SHIFT;
const int32 HALF_MIN_NORM = HALF_MIN_NORM_AS_32 >> HALF_SHIFT;
const int32 HALF_SIGN_BIT = FLOAT_SIGN_BIT >> HALF_SHIFT_SIGN; // flt16 sign bit
const int32 FLOAT_MAX_SUBNORM_AS_16 = 0x003FF; // max flt32 subnormal down shifted
const int32 FLOAT_MIN_NORM_AS_16 = 0x00400; // min flt32 normal down shifted
const int32 HALF_MAX_DENORM = (FLOAT_POS_INF >> HALF_SHIFT) - HALF_MAX_NORM - 1;
const int32 HALF_MIN_DENORM = HALF_MIN_NORM - FLOAT_MAX_SUBNORM_AS_16 - 1;

} // namespace anonymous


// float <-> half float conversions taken from: http://stackoverflow.com/a/3542975

HalfFloat ToHalfFloat(float value)
{
    Bits32 v, s;
    v.f = value;
    uint32 sign = v.si & FLOAT_SIGN_BIT;
    v.si ^= sign;
    sign >>= HALF_SHIFT_SIGN; // logical HALF_SHIFT
    s.si = 0x52000000; // (1 << 23) / HALF_MIN_NORM_AS_32
    s.si = static_cast<int32>(s.f * v.f); // correct subnormals
    v.si ^= (s.si ^ v.si) & -(HALF_MIN_NORM_AS_32 > v.si);
    v.si ^= (FLOAT_POS_INF ^ v.si) & -((FLOAT_POS_INF > v.si) & (v.si > HALF_MAX_NORM_AS_32));
    v.si ^= (HALF_MIN_NAN_AS_32 ^ v.si) & -((HALF_MIN_NAN_AS_32 > v.si) & (v.si > FLOAT_POS_INF));
    v.ui >>= HALF_SHIFT; // logical HALF_SHIFT
    v.si ^= ((v.si - HALF_MAX_DENORM) ^ v.si) & -(v.si > HALF_MAX_NORM);
    v.si ^= ((v.si - HALF_MIN_DENORM) ^ v.si) & -(v.si > FLOAT_MAX_SUBNORM_AS_16);
    return static_cast<HalfFloat>(v.ui | sign);
}

float ConvertHalfFloatToFloat(HalfFloat value)
{
    Bits32 v, s;
    v.ui = value;
    int32_t sign = v.si & HALF_SIGN_BIT;
    v.si ^= sign;
    sign <<= HALF_SHIFT_SIGN;
    v.si ^= ((v.si + HALF_MIN_DENORM) ^ v.si) & -(v.si > FLOAT_MAX_SUBNORM_AS_16);
    v.si ^= ((v.si + HALF_MAX_DENORM) ^ v.si) & -(v.si > HALF_MAX_NORM);
    s.si = 0x33800000; // HALF_MIN_NORM_AS_32 / (1 << (23 - HALF_SHIFT))
    s.f *= v.si;
    int32_t mask = -(FLOAT_MIN_NORM_AS_16 > v.si);
    v.si <<= HALF_SHIFT;
    v.si ^= (s.si ^ v.si) & mask;
    v.si |= sign;
    return v.f;
}


uint8 ToUint8(float x)
{
    return static_cast<uint8>(Math::Clamp(x, 0.0f, 255.0f));
}

uint8 ToNormUint8(float x)
{
    return static_cast<uint8>(255.0f * Math::Clamp(x, 0.0f, 1.0f) + 0.5f);
}

int8 ToInt8(float x)
{
    return static_cast<int8>(Math::Clamp(x, -128.0f, 127.0f));
}

int8 ToNormInt8(float x)
{
    if (x > 1.0f)
        return INT8_MAX;
    else if (x < -1.0f)
        return INT8_MIN;

    x = static_cast<float>(INT8_MAX) * x;
    if (x > 0.5f)
        x += 0.5f;
    else if (x < 0.5f)
        x -= 0.5f;

    return static_cast<int8>(x);
}

uint16 ToUint16(float x)
{
    return static_cast<uint16>(Math::Clamp(x, 0.0f, static_cast<float>(UINT16_MAX)));
}

uint16 ToNormUint16(float x)
{
    return static_cast<uint16>(static_cast<float>(UINT16_MAX) * Math::Clamp(x, 0.0f, 1.0f) + 0.5f);
}

int16 ToInt16(float x)
{
    return static_cast<int16>(Math::Clamp(x, static_cast<float>(INT16_MIN), static_cast<float>(INT16_MAX)));
}

int16 ToNormInt16(float x)
{
    if (x > 1.0f)
        return INT16_MAX;
    else if (x < -1.0f)
        return INT16_MIN;

    x = static_cast<float>(INT16_MAX) * x;
    if (x > 0.5f)
        x += 0.5f;
    else if (x < 0.5f)
        x -= 0.5f;

    return static_cast<int16>(x);
}

uint32 ToUint32(float x)
{
    if (x >= static_cast<float>(UINT32_MAX))
        return UINT32_MAX;
    else if (x <= 0.0f)
        return 0;
    else
        return static_cast<int32>(x);
}

int32 ToInt32(float x)
{
    if (x >= static_cast<float>(INT32_MAX))
        return INT32_MAX;
    else if (x <= static_cast<float>(INT32_MIN))
        return INT32_MIN;
    else
        return static_cast<int32>(x);
}

} // namespace Math
} // namespace NFE
