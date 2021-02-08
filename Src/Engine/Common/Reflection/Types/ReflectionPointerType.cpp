/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionPointerType.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Config/ConfigValue.hpp"

namespace NFE {
namespace RTTI {

using namespace Common;

PointerType::PointerType(const Type* underlyingType)
    : mUnderlyingType(underlyingType)
{
    NFE_ASSERT(mUnderlyingType, "Invalid pointed type");
}

bool PointerType::Compare(const void* objectA, const void* objectB) const
{
    const void* pointedObjectA = GetPointedData(objectA);
    const void* pointedObjectB = GetPointedData(objectB);

    if (pointedObjectA && pointedObjectB)
    {
        const Type* pointedTypeA = GetPointedDataType(objectA);
        const Type* pointedTypeB = GetPointedDataType(objectB);

        NFE_ASSERT(pointedTypeA, "Invalid type");
        NFE_ASSERT(pointedTypeB, "Invalid type");

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
    const Type* pointedSrcType = GetPointedDataType(sourceObject);

    Reset(destObject, pointedSrcType);

    if (pointedSrcType)
    {
        void* pointedDestObject = GetPointedData(destObject);
        const void* pointedSrcObject = GetPointedData(sourceObject);

        NFE_ASSERT(pointedDestObject, "Invalid object");
        NFE_ASSERT(pointedSrcObject, "Invalid object");

        return pointedSrcType->Clone(pointedDestObject, pointedSrcObject);
    }

    return true;
}

bool PointerType::SerializeTypeName(OutputStream* stream, SerializationContext& context) const
{
    // write header
    if (!Type::SerializeTypeName(stream, context))
    {
        return false;
    }

    // append inner type
    return mUnderlyingType->SerializeTypeName(stream, context);
}

} // namespace RTTI
} // namespace NFE
