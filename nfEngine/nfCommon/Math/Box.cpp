/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Box class definitions.
 */

#include "PCH.hpp"
#include "Box.hpp"


namespace NFE {
namespace Math {

float Box::SurfaceArea() const
{
    if (IsEmpty())
    {
        return 0.0f;
    }

    const Vector size = max - min;
    return 2.0f * (size.x * (size.y + size.z) + size.y * size.z);
}

float Box::Volume() const
{
    if (IsEmpty())
    {
        return 0.0f;
    }

    const Vector size = max - min;
    return size.x * size.y * size.z;
}

} // namespace Math
} // namespace NFE
