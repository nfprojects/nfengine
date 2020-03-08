/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionPointerType.hpp"

namespace NFE {
namespace RTTI {

bool PointerType::Compare(const void* objectA, const void* objectB) const
{
    const void* pointedObjectA = GetPointedData(objectA);
    const void* pointedObjectB = GetPointedData(objectB);

    if (pointedObjectA && pointedObjectB)
    {
        const Type* pointedTypeA = GetPointedType(objectA);
        const Type* pointedTypeB = GetPointedType(objectB);

        NFE_ASSERT(pointedTypeA);
        NFE_ASSERT(pointedTypeB);

        if (pointedTypeA != pointedTypeB)
        {
            return false;
        }

        return pointedTypeA->Compare(pointedObjectA, pointedObjectB);
    }

    if (!pointedObjectA && !pointedObjectB)
    {
        // both pointers are null - it means they are the same
        return true;
    }

    // one pointer is null, the other is not
    return false;
}

bool PointerType::Clone(void* destObject, const void* sourceObject) const
{
    const Type* pointedSrcType = GetPointedType(sourceObject);

    Reset(destObject, pointedSrcType);

    if (pointedSrcType)
    {
        void* pointedDestObject = GetPointedData(destObject);
        const void* pointedSrcObject = GetPointedData(sourceObject);

        NFE_ASSERT(pointedDestObject);
        NFE_ASSERT(pointedSrcObject);

        return pointedSrcType->Clone(pointedDestObject, pointedSrcObject);
    }

    return true;
}

} // namespace RTTI
} // namespace NFE
