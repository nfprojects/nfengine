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
    Member(const char* name, size_t offset, const Type* type)
        : mName(name)
        , mType(type)
        , mOffset(offset)
    { }

    const char* GetName() const { return mName; }
    const Type* GetType() const { return mType; }
    size_t GetOffset() const { return mOffset; }

private:
    const char* mName;      // member name
    const Type* mType;      // member type
    size_t mOffset;         // offset from the beginning of the class (in bytes)

    // TODO flags and tags
    // TODO editor support
};

} // namespace RTTI
} // namespace NFE
