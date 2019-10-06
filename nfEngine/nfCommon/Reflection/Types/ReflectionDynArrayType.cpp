/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionDynArrayType.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

void DynArrayType::PrintInfo() const
{
    Type::PrintInfo();
}

uint32 DynArrayType::GetArraySize(const void* arrayObject) const
{
    // HACK: assumes DynArray layout does not depend on underlying type
    const DynArray<uint32>& typedObject = *static_cast<const DynArray<uint32>*>(arrayObject);
    return typedObject.Size();
}

bool DynArrayType::Compare(const void* objectA, const void* objectB) const
{
    const uint32 arraySizeA = GetArraySize(objectA);
    const uint32 arraySizeB = GetArraySize(objectB);
    if (arraySizeA != arraySizeB)
    {
        return false;
    }

    const Type* underlyingType = GetUnderlyingType();
    for (uint32 i = 0; i < arraySizeA; ++i)
    {
        if (!underlyingType->Compare(GetElementPointer(objectA, i), GetElementPointer(objectB, i)))
        {
            return false;
        }
    }

    return true;
}

} // namespace RTTI
} // namespace NFE
