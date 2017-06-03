/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Rect (rectangle) class definition.
 */

#pragma once

#include "Math.hpp"


namespace NFE {
namespace Math {

/**
 * Rectangle template.
 */
template<typename T>
struct Rect
{
    T Xmin, Xmax, Ymin, Ymax;

    Rect() : Xmin(0), Xmax(0), Ymin(0), Ymax(0) { }

    explicit Rect(const T& Xmin_, const T& Ymin_, const T& Xmax_, const T& Ymax_)
        : Xmin(Xmin_), Xmax(Xmax_), Ymin(Ymin_), Ymax(Ymax_) { }
};

using Recti = Rect<int>;
using Rectf = Rect<float>;


} // namespace Math
} // namespace NFE
