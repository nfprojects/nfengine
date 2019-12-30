/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionNativeArrayType.hpp"


namespace NFE {
namespace RTTI {


void NativeArrayType::PrintInfo() const
{
    Type::PrintInfo();
    NFE_LOG_DEBUG("  array size = %u", mArraySize);
}

void* NativeArrayType::GetElementPointer(void* arrayData, uint32 index) const
{
    NFE_ASSERT(index < mArraySize);
    return reinterpret_cast<uint8*>(arrayData) + index * GetUnderlyingType()->GetSize();
}

const void* NativeArrayType::GetElementPointer(const void* arrayData, uint32 index) const
{
    NFE_ASSERT(index < mArraySize);
    return reinterpret_cast<const uint8*>(arrayData) + index * GetUnderlyingType()->GetSize();
}

bool NativeArrayType::Compare(const void* objectA, const void* objectB) const
{
    const Type* underlyingType = GetUnderlyingType();

    for (uint32 i = 0; i < mArraySize; ++i)
    {
        if (!underlyingType->Compare(GetElementPointer(objectA, i), GetElementPointer(objectB, i)))
        {
            return false;
        }
    }

    return true;
}

bool NativeArrayType::Clone(void* destObject, const void* sourceObject) const
{
    const Type* underlyingType = GetUnderlyingType();

    bool success = true;

    for (uint32 i = 0; i < mArraySize; ++i)
    {
        if (!underlyingType->Clone(GetElementPointer(destObject, i), GetElementPointer(sourceObject, i)))
        {
            success = false;
        }
    }

    return success;
}

} // namespace RTTI
} // namespace NFE
