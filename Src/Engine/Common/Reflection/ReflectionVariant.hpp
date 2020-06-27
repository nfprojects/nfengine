#pragma once

#include "ReflectionTypeResolver.hpp"
#include "Types/ReflectionType.hpp"
#include "../Memory/Buffer.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace RTTI {

/**
 * Hold an object of any RTTI type.
 * Note: the variant has ownership of the object - it's destroyed when the variant is destroyed.
 */
class NFCOMMON_API Variant
{
public:
    Variant();
    ~Variant();
    Variant(Variant&& other);
    Variant(const Type* dataType, Common::Buffer&& data);
    Variant& operator = (Variant&& other);

    // Reset variant state (make it null)
    void Clear();

    NFE_FORCE_INLINE bool Empty() const { return mDataType == nullptr; }
    NFE_FORCE_INLINE const Type* GetType() const { return mDataType; }
    NFE_FORCE_INLINE void* GetData() const { return mObjectData.Data(); }
    NFE_FORCE_INLINE const Common::Buffer& GetDataBuffer() const { return mObjectData; }

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

private:
    const Type* mDataType;
    Common::Buffer mObjectData;
};

} // namespace RTTI
} // namespace NFE
