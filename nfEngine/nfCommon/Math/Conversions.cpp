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

const float UINT8_MAX_FLOAT = static_cast<float>(UINT8_MAX);
const float UINT16_MAX_FLOAT = static_cast<float>(UINT16_MAX);
const float UINT32_MAX_FLOAT = static_cast<float>(UINT32_MAX);
const float INT8_MIN_FLOAT = static_cast<float>(INT8_MIN);
const float INT16_MIN_FLOAT = static_cast<float>(INT16_MIN);
const float INT32_MIN_FLOAT = static_cast<float>(INT32_MIN);
const float INT8_MAX_FLOAT = static_cast<float>(INT8_MAX);
const float INT16_MAX_FLOAT = static_cast<float>(INT16_MAX);
const float INT32_MAX_FLOAT = static_cast<float>(INT32_MAX);

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

float ToFloat(HalfFloat value)
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
    return static_cast<uint8>(Math::Clamp(x, 0.0f, UINT8_MAX_FLOAT));
}

uint8 ToNormUint8(float x)
{
    return static_cast<uint8>(UINT8_MAX_FLOAT * Math::Clamp(x, 0.0f, 1.0f) + 0.5f);
}

int8 ToInt8(float x)
{
    return static_cast<int8>(Math::Clamp(x, INT8_MIN_FLOAT, INT8_MAX_FLOAT));
}

int8 ToNormInt8(float x)
{
    if (x > 1.0f)
        return INT8_MAX;
    else if (x < -1.0f)
        return INT8_MIN;

    x = INT8_MAX_FLOAT * x;
    if (x > 0.5f)
        x += 0.5f;
    else if (x < 0.5f)
        x -= 0.5f;

    return static_cast<int8>(x);
}

uint16 ToUint16(float x)
{
    return static_cast<uint16>(Math::Clamp(x, 0.0f, UINT16_MAX_FLOAT));
}

uint16 ToNormUint16(float x)
{
    return static_cast<uint16>(UINT16_MAX_FLOAT * Math::Clamp(x, 0.0f, 1.0f) + 0.5f);
}

int16 ToInt16(float x)
{
    return static_cast<int16>(Math::Clamp(x, INT16_MIN_FLOAT, INT16_MAX_FLOAT));
}

int16 ToNormInt16(float x)
{
    if (x > 1.0f)
        return INT16_MAX;
    else if (x < -1.0f)
        return INT16_MIN;

    x = INT16_MAX_FLOAT * x;
    if (x > 0.5f)
        x += 0.5f;
    else if (x < 0.5f)
        x -= 0.5f;

    return static_cast<int16>(x);
}

uint32 ToUint32(float x)
{
    if (x >= UINT32_MAX_FLOAT)
        return UINT32_MAX;
    else if (x <= 0.0f)
        return 0;
    else
        return static_cast<int32>(x);
}

int32 ToInt32(float x)
{
    if (x >= INT32_MAX_FLOAT)
        return INT32_MAX;
    else if (x <= INT32_MIN_FLOAT)
        return INT32_MIN;
    else
        return static_cast<int32>(x);
}

} // namespace Math
} // namespace NFE
