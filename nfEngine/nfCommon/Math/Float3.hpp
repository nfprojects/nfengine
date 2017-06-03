/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Float3 class definition.
 */

#pragma once

#include "Math.hpp"
#include "Float2.hpp"


namespace NFE {
namespace Math {


/**
 * Structure for efficient 3D vector storing.
 */
struct Float3
{
    float x;
    float y;
    float z;

    constexpr Float3() : x(0.0f), y(0.0f), z(0.0f) { }
    constexpr explicit Float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) { }
    constexpr explicit Float3(float* array) : x(array[0]), y(array[1]), z(array[2]) { }
};



} // namespace Math
} // namespace NFE
