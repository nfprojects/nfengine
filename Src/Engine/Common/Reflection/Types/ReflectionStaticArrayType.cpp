/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionStaticArrayType.hpp"
#include "ReflectionNativeArrayType.hpp"
#include "../SerializationContext.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

// hacky type punning - this structure must match Common::StaticArray layout
using StaticArrayAccessor = StaticArray<char,1>;

const Common::String StaticArrayType::BuildTypeName(const Type* underlyingType, uint32 capacity)
{
    NFE_ASSERT(underlyingType, "Invalid type");
    return Common::String::Printf("StaticArray<%s,%u>", underlyingType->GetName().Str(), capacity);
}

StaticArrayType::StaticArrayType(const Type* underlyingType, uint32 capacity)
    : ResizableArrayType(underlyingType)
    , mCapacity(capacity)
{ }

void StaticArrayType::PrintInfo() const
{
    Type::PrintInfo();
}

uint32 StaticArrayType::GetArraySize(const void* arrayObject) const
{
    const StaticArrayAccessor& typedObject = *static_cast<const StaticArrayAccessor*>(arrayObject);
    return typedObject.Size();
}

uint32 StaticArrayType::GetMaxCapacity() const
{
    return mCapacity;
}

bool StaticArrayType::ResizeArray(void* arrayObject, uint32 targetSize) const
{
    NFE_ASSERT(arrayObject, "Invalid array object");
    NFE_ASSERT(targetSize <= mCapacity, "Target size (%u) exceedes array capacity (%u)", targetSize, mCapacity);

    StaticArrayAccessor* accessor = BitCast<StaticArrayAccessor*>(arrayObject);
    const Type* elementType = GetUnderlyingType();
    const size_t objectSize = elementType->GetSize();

    const uint32 oldSize = accessor->mSize;

    // call destructors
    for (uint32 i = targetSize; i < oldSize; ++i)
    {
        elementType->DestructObject(accessor->Data() + i * objectSize);
    }

    // initialize new elements
    for (uint32 i = oldSize; i < targetSize; ++i)
    {
        elementType->ConstructObject(accessor->Data() + i * objectSize);
    }

    accessor->mSize = targetSize;
    return true;
}

void* StaticArrayType::GetElementPointer(void* arrayObject, uint32 index) const
{
    NFE_ASSERT(arrayObject, "Invalid array object");

    StaticArrayAccessor* accessor = BitCast<StaticArrayAccessor*>(arrayObject);
    NFE_ASSERT(index < accessor->mSize, "DynArray index out of bounds. Index=%u, Size=%u", index, accessor->mSize);

    return accessor->Data() + index * GetUnderlyingType()->GetSize();
}

const void* StaticArrayType::GetElementPointer(const void* arrayObject, uint32 index) const
{
    NFE_ASSERT(arrayObject, "Invalid array object");

    const StaticArrayAccessor* accessor = BitCast<const StaticArrayAccessor*>(arrayObject);
    NFE_ASSERT(index < accessor->mSize, "DynArray index out of bounds. Index=%u, Size=%u", index, accessor->mSize);

    return accessor->Data() + index * GetUnderlyingType()->GetSize();
}

} // namespace RTTI
} // namespace NFE
