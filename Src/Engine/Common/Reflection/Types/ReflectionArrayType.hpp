/**
 * @file
 * @author Witek902
 */

#pragma once

#include "ReflectionType.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for generic array (native, DynArray, StaticArray, etc.)
 */
class NFCOMMON_API IArrayType : public Type
{
    NFE_MAKE_NONCOPYABLE(IArrayType)

public:
    NFE_FORCE_INLINE IArrayType(const Type* underlyingType)
        : mUnderlyingType(underlyingType)
    {
        NFE_ASSERT(mUnderlyingType, "Array's underlying type cannot be null. Type: %s", GetName().Str());
    }

    // get number of array elements
    virtual uint32 GetArraySize(const void* arrayObject) const = 0;

    // get type of the array element
    NFE_FORCE_INLINE const Type* GetUnderlyingType() const { return mUnderlyingType; }

    // access element data
    virtual void* GetElementPointer(void* arrayObject, uint32 index) const = 0;
    virtual const void* GetElementPointer(const void* arrayObject, uint32 index) const = 0;

protected:
    const Type* mUnderlyingType;    // array element type
};

} // namespace RTTI
} // namespace NFE
