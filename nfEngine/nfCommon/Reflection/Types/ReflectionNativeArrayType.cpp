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

} // namespace RTTI
} // namespace NFE
