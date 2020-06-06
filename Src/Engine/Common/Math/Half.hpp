#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {


// half (16-bit) floating point type
class Half
{
public:
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

private:
    NFE_UNNAMED_STRUCT union
    {
        uint16 value;

        NFE_UNNAMED_STRUCT struct
        {
            uint16 mantissa : 10;
            uint16 exponent : 5;
            uint16 sign : 1;
        } components;
    };
};


struct Half2
{
    NFE_FORCE_INLINE Half2() = default;
    NFE_FORCE_INLINE Half2(const Half2& other) : packed(other.packed) { }

    NFE_FORCE_INLINE Half2& operator = (const Half2& other)
    {
        packed = other.packed;
        return *this;
    }

    NFE_UNNAMED_STRUCT union
    {
        uint32 packed;

        NFE_UNNAMED_STRUCT struct
        {
            Half x;
            Half y;
        };
    };
};


struct Half3
{
    Half x;
    Half y;
    Half z;
};


struct Half4
{
    NFE_FORCE_INLINE Half4() = default;
    NFE_FORCE_INLINE Half4(const Half4& other) : packed(other.packed) { }
    
    NFE_FORCE_INLINE Half4& operator = (const Half4& other)
    {
        packed = other.packed;
        return *this;
    }

    NFE_UNNAMED_STRUCT union
    {
        uint64 packed;

        NFE_UNNAMED_STRUCT struct
        {
            Half x;
            Half y;
            Half z;
            Half w;
        };
    };
};


} // namespace Math
} // namespace NFE


#include "HalfImpl.hpp"
