/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Member class.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace RTTI {

class Type;

/**
 * Class member information.
 */
class NFCOMMON_API Member
{
public:
    NFE_INLINE Member(const char* name, size_t offset, const Type* type)
        : mName(name)
        , mOffset(offset)
        , mType(type)
    { }

    NFE_INLINE const std::string& GetName() const { return mName; }
    NFE_INLINE const Type* GetType() const { return mType; }
    NFE_INLINE size_t GetOffset() const { return mOffset; }

private:
    std::string mName;      // member name
    const Type* mType;      // member type
    size_t mOffset;         // offset from the beginning of the class (in bytes)

    // TODO flags and tags
};

} // namespace RTTI
} // namespace NFE
