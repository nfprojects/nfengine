/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionUniquePtrType.hpp"

namespace NFE {
namespace RTTI {

bool UniquePtrType::Compare(const void* objectA, const void* objectB) const
{
    const void* pointedObjectA = GetPointedData(objectA);
    const void* pointedObjectB = GetPointedData(objectB);

    if (pointedObjectA && pointedObjectB)
    {
        return GetPointedType()->Compare(pointedObjectA, pointedObjectB);
    }

    if (!pointedObjectA && !pointedObjectB)
    {
        // both pointers are null - it means they are the same
        return true;
    }

    // one pointer is null, the other is not
    return false;
}

} // namespace RTTI
} // namespace NFE
