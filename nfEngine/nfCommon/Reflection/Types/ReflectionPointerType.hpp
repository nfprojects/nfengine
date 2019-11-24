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
    NFE_FORCE_INLINE const Type* GetPointedType() const { return mPointedType; }

    // get data pointed by the unique pointer
    virtual void* GetPointedData(const void* uniquePtrObject) const = 0;

    // get object type under the pointer
    virtual const Type* GetPointedType(const void* uniquePtrObject) const = 0;

    // set object under the pointer
    virtual void Reset(void* uniquePtrObject, const Type* newDataType = nullptr) const = 0;

    virtual bool Compare(const void* objectA, const void* objectB) const override;

protected:
    NFE_FORCE_INLINE PointerType(const TypeInfo& info) : Type(info) { }

    const Type* mPointedType;
};

} // namespace RTTI
} // namespace NFE
