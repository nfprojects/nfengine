#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {


// half (16-bit) floating point type
union Half
{
    NFE_FORCE_INLINE Half() = default;

    NFE_FORCE_INLINE Half(const Half& other) : value(other.value) { }
    NFE_FORCE_INLINE explicit Half(const uint16 other) : value(other) { }

    NFE_FORCE_INLINE Half& operator = (const Half& other)
    {
        value = other.value;
        return *this;
    }

    // convert 32-bit float to half
    NFE_INLINE Half(const float other);

    // convert to 32-bit float
    NFE_INLINE float ToFloat() const;

    // check if not NaN or infinity
    NFE_INLINE bool IsValid() const;

    NFE_UNNAMED_STRUCT struct
    {
        uint16 mantissa : 10;
        uint16 exponent : 5;
        uint16 sign : 1;
    } components;

    uint16 value;
};


struct Half2
{
    Half x;
    Half y;
};


struct Half3
{
    Half x;
    Half y;
    Half z;
};


struct Half4
{
    Half x;
    Half y;
    Half z;
    Half w;
};


} // namespace Math
} // namespace NFE


#include "HalfImpl.hpp"
