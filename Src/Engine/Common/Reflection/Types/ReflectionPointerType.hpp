/**
 * @file
 * @brief  Definition of PointerType.
 */

#pragma once

#include "ReflectionType.hpp"

namespace NFE {
namespace RTTI {

/**
 * Type information for UniquePtr<T> or SharedPtr<T> types.
 */
class NFCOMMON_API PointerType : public Type
{
    NFE_MAKE_NONCOPYABLE(PointerType)

public:
    PointerType(const Type* underlyingType);

    NFE_FORCE_INLINE const Type* GetUnderlyingType() const { return mUnderlyingType; }

    // get data pointed by the unique pointer
    virtual void* GetPointedData(const void* ptrObject) const = 0;

    // get object type under the pointer
    virtual const Type* GetPointedDataType(const void* uniquePtrObject) const = 0;

    // set object under the pointer and return its pointer
    virtual void* Reset(void* ptrObject, const Type* newDataType = nullptr) const = 0;

    virtual bool Compare(const void* objectA, const void* objectB) const override;
    virtual bool Clone(void* destObject, const void* sourceObject) const override;
    virtual bool SerializeTypeName(Common::OutputStream* stream, SerializationContext& context) const override;

protected:
    const Type* mUnderlyingType;
};

} // namespace RTTI
} // namespace NFE
