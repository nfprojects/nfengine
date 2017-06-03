/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Float4 class definition.
 */

#pragma once

#include "Math.hpp"
#include "Float3.hpp"


namespace NFE {
namespace Math {


/**
 * Structure for efficient 4D vector storing - unaligned version of Vector class.
 */
struct Float4
{
    float x;
    float y;
    float z;
    float w;

    constexpr Float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    constexpr explicit Float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    constexpr explicit Float4(float* array) : x(array[0]), y(array[1]), z(array[2]), w(array[3]) {}
};



} // namespace Math
} // namespace NFE
