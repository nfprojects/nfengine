/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of functions for types conversions.
 */

#include "PCH.hpp"
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

} // namespace anonymous


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
