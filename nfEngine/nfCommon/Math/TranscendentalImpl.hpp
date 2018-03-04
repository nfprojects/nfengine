/**
 * @file
 * @brief  Definitions of transcendental functions.
 */

#pragma once

#include "Transcendental.hpp"


namespace NFE {
namespace Math {


float Tan(float x)
{
    return Sin(x) / Cos(x);
}

float Cot(float x)
{
    return Cos(x) / Sin(x);
}


} // namespace Math
} // namespace NFE
