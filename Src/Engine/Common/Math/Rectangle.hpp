#pragma once

#include "Math.hpp"

namespace NFE {
namespace Math {

// 2D rectangle
template<typename T>
class Rectangle
{
public:
    NFE_FORCE_INLINE constexpr Rectangle()
        : minX(T(0))
        , maxX(T(0))
        , minY(T(0))
        , maxY(T(0))
    { }

    NFE_FORCE_INLINE constexpr Rectangle(T minX, T maxX, T minY, T maxY)
        : minX(minX)
        , maxX(maxX)
        , minY(minY)
        , maxY(maxY)
    { }

    NFE_FORCE_INLINE constexpr T Width() const
    {
        return maxX - minX;
    }

    NFE_FORCE_INLINE constexpr T Height() const
    {
        return maxY - minY;
    }

    T minX;
    T maxX;
    T minY;
    T maxY;
};

using Rectf = Rectangle<float>;
using Recti = Rectangle<int32>;

} // namespace Math
} // namespace NFE
