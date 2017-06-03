/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Float2 class definition.
 */

#pragma once

#include "Math.hpp"


namespace NFE {
namespace Math {


/**
 * Structure for efficient 2D vector storing.
 */
struct Float2
{
    float x;
    float y;

    constexpr Float2() : x(0.0f), y(0.0f) { }
    constexpr explicit Float2(float x_, float y_) : x(x_), y(y_) { }
    constexpr explicit Float2(float* array) : x(array[0]), y(array[1]) { }
};



} // namespace Math
} // namespace NFE
