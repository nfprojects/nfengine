#include "PCH.hpp"
#include "ReflectionVariant.hpp"
#include "Types/ReflectionType.hpp"


namespace NFE {
namespace RTTI {


Variant::Variant()
    : mDataType(nullptr)
{}

Variant::~Variant()
{
    if (mDataType)
    {
        mDataType->DestructObject(mObjectData.Data());
    }
}

Variant::Variant(Variant&& other)
    : mDataType(other.mDataType)
    , mObjectData(std::move(other.mObjectData))
{
    other.mDataType = nullptr;
    NFE_ASSERT(other.mObjectData.Data() == nullptr, "Other object data should have been moved");
}

Variant::Variant(const Type* dataType, Common::StaticBuffer&& data)
    : mDataType(dataType)
    , mObjectData(std::move(data))
{
    if (mDataType)
    {
        NFE_ASSERT(mDataType->GetSize() == mObjectData.Size(), "Type size (%zu) and object size (%zu) do not match", mDataType->GetSize(), mObjectData.Size());
    }
}

Variant& Variant::operator = (Variant&& other)
{
    Clear();

    mDataType = other.mDataType;
    mObjectData = std::move(other.mObjectData);

    other.mDataType = nullptr;
    other.mObjectData.Release();

    return *this;
}

void Variant::Clear()
{
    if (mDataType)
    {
        mDataType->DestructObject(mObjectData.Data());
        mDataType = nullptr;
    }
    
    mObjectData.Release();
}

} // namespace RTTI
} // namespace NFE
