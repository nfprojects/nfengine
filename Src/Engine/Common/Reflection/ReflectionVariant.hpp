#pragma once

#include "ReflectionTypeResolver.hpp"
#include "Types/ReflectionType.hpp"
#include "../Memory/StaticBuffer.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace RTTI {

/**
 * Holds a read-only reference to an object of any RTTI type.
 * Note: the variant view does not keep reference - it behaves like StringView or ArrayView.
 */
class NFCOMMON_API VariantView
{
public:
    VariantView() = default;
    VariantView(const Type* dataType, const void* data) : mDataType(dataType), mObjectData(data) { }

    NFE_FORCE_INLINE bool Empty() const { return mDataType == nullptr; }
    NFE_FORCE_INLINE const Type* GetType() const { return mDataType; }
    NFE_FORCE_INLINE const void* GetData() const { return mObjectData; }

    // Access hold object by reference (the object must exist)
    template<typename T>
    const T& Get() const
    {
        NFE_ASSERT(mDataType, "Variant is empty");
        NFE_ASSERT(RTTI::GetType<T>()->IsA(mDataType), "Incompatible type");
        return *reinterpret_cast<const T*>(mObjectData);
    }

    // Access hold object by pointer (may return nullptr if object does not exist)
    template<typename T>
    const T* GetPtr() const
    {
        if (mDataType)
        {
            NFE_ASSERT(RTTI::GetType<T>()->IsA(mDataType), "Incompatible type");
            return reinterpret_cast<const T*>(mObjectData);
        }
        return nullptr;
    }

private:
    const Type* mDataType = nullptr;
    const void* mObjectData = nullptr;
};

/**
 * Holds an object of any RTTI type.
 * Note: the variant has ownership of the object - it's destroyed when the variant is destroyed.
 */
class NFCOMMON_API Variant
{
public:
    Variant();
    ~Variant();
    Variant(Variant&& other);
    Variant(const Type* dataType, Common::StaticBuffer&& data);
    Variant& operator = (Variant&& other);

    // Reset variant state (make it null)
    void Clear();

    NFE_FORCE_INLINE bool Empty() const { return mDataType == nullptr; }
    NFE_FORCE_INLINE const Type* GetType() const { return mDataType; }
    NFE_FORCE_INLINE void* GetData() const { return mObjectData.Data(); }
    NFE_FORCE_INLINE const Common::StaticBuffer& GetDataBuffer() const { return mObjectData; }

    // Access hold object by reference (the object must exist)
    template<typename T>
    T& Get() const
    {
        NFE_ASSERT(mDataType, "Variant is empty");
        NFE_ASSERT(RTTI::GetType<T>()->IsA(mDataType), "Incompatible type");
        return *reinterpret_cast<T*>(mObjectData.Data());
    }

    // Access hold object by pointer (may return nullptr if object does not exist)
    template<typename T>
    T* GetPtr() const
    {
        if (mDataType)
        {
            NFE_ASSERT(RTTI::GetType<T>()->IsA(mDataType), "Incompatible type");
            return reinterpret_cast<T*>(mObjectData.Data());
        }
        return nullptr;
    }

    NFE_FORCE_INLINE const VariantView ToView() const
    {
        return VariantView{ GetType(), GetData() };
    }

private:
    const Type* mDataType;
    Common::StaticBuffer mObjectData;
};

} // namespace RTTI
} // namespace NFE
