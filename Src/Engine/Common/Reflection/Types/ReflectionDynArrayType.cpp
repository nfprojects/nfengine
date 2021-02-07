/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionDynArrayType.hpp"
#include "ReflectionNativeArrayType.hpp"
#include "../SerializationContext.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

// hacky type punning - this structure must match Common:DynArray layout
using DynArrayAccessor = DynArray<char>;

const Common::String DynArrayType::BuildTypeName(const Type* underlyingType)
{
    NFE_ASSERT(underlyingType, "Invalid type");
    return Common::String("DynArray<") + underlyingType->GetName() + '>';
}

void DynArrayType::PrintInfo() const
{
    Type::PrintInfo();
}

uint32 DynArrayType::GetArraySize(const void* arrayObject) const
{
    const DynArrayAccessor& typedObject = *static_cast<const DynArrayAccessor*>(arrayObject);
    return typedObject.Size();
}

bool DynArrayType::ReserveArray(void* arrayObject, uint32 targetCapacity) const
{
    NFE_ASSERT(arrayObject, "Invalid array object");

    DynArrayAccessor* accessor = BitCast<DynArrayAccessor*>(arrayObject);
    const Type* elementType = GetUnderlyingType();
    const size_t objectSize = elementType->GetSize();

    if (targetCapacity <= accessor->mAllocSize)
    {
        // smaller that allocated - ignore
        return true;
    }

    uint32 newCapacity = accessor->mAllocSize;
    while (targetCapacity > newCapacity)
    {
        // grow by 50%
        newCapacity += Math::Max<uint32>(1, newCapacity / 2);
    }

    char* newBuffer = (char*)NFE_MALLOC(newCapacity * objectSize, elementType->GetAlignment());
    if (!newBuffer)
    {
        // memory allocation failed
        return false;
    }

    // TODO move constructor
    // move objects
    for (uint32 i = 0; i < accessor->mSize; ++i)
    {
        elementType->ConstructObject(newBuffer + i * objectSize);
        elementType->Clone(newBuffer + i * objectSize, accessor->mElements + i * objectSize);
        elementType->DestructObject(accessor->mElements + i * objectSize);
    }

    // replace buffer
    NFE_FREE(accessor->mElements);
    accessor->mElements = (char*)newBuffer;
    accessor->mAllocSize = newCapacity;
    return true;
}

bool DynArrayType::ResizeArray(void* arrayObject, uint32 targetSize) const
{
    NFE_ASSERT(arrayObject, "Invalid array object");

    DynArrayAccessor* accessor = BitCast<DynArrayAccessor*>(arrayObject);
    const Type* elementType = GetUnderlyingType();
    const size_t objectSize = elementType->GetSize();

    const uint32 oldSize = accessor->mSize;

    // call destructors
    for (uint32 i = targetSize; i < oldSize; ++i)
    {
        elementType->DestructObject(accessor->mElements + i * objectSize);
    }

    if (!ReserveArray(arrayObject, targetSize))
    {
        return false;
    }

    // initialize new elements
    for (uint32 i = oldSize; i < targetSize; ++i)
    {
        elementType->ConstructObject(accessor->mElements + i * objectSize);
    }

    accessor->mSize = targetSize;
    return true;
}

void* DynArrayType::GetElementPointer(void* arrayObject, uint32 index) const
{
    NFE_ASSERT(arrayObject, "Invalid array object");

    const DynArrayAccessor* accessor = BitCast<const DynArrayAccessor*>(arrayObject);
    NFE_ASSERT(index < accessor->mSize, "DynArray index out of bounds. Index=%u, Size=%u", index, accessor->mSize);

    return accessor->mElements + index * GetUnderlyingType()->GetSize();
}

const void* DynArrayType::GetElementPointer(const void* arrayObject, uint32 index) const
{
    NFE_ASSERT(arrayObject, "Invalid array object");

    const DynArrayAccessor* accessor = BitCast<const DynArrayAccessor*>(arrayObject);
    NFE_ASSERT(index < accessor->mSize, "DynArray index out of bounds. Index=%u, Size=%u", index, accessor->mSize);

    return accessor->mElements + index * GetUnderlyingType()->GetSize();
}

} // namespace RTTI
} // namespace NFE
