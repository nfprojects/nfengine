/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Member class.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace RTTI {

/**
 * Class member information.
 */
class NFCOMMON_API Member
{
public:
    NFE_INLINE Member(const char* name, size_t offset, const Type* type, size_t arraySize = 0)
        : mName(name)
        , mOffset(offset)
        , mType(type)
        , mArraySize(arraySize)
    { }

    NFE_INLINE const char* GetName() const { return mName; }
    NFE_INLINE const Type* GetType() const { return mType; }
    NFE_INLINE size_t GetOffset() const { return mOffset; }

    NFE_INLINE bool IsArray() const { return mArraySize != 0; }
    NFE_INLINE size_t GetArraySize() const { return mArraySize; }

private:
    const char* mName;      // member name
    const Type* mType;      // member type
    size_t mOffset;         // offset from the beginning of the class (in bytes)
    size_t mArraySize;      // if non-zero it means that the member is an array of objects

    // TODO flags and tags
};

} // namespace RTTI
} // namespace NFE
