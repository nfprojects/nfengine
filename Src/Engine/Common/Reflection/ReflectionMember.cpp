/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of reflection system's Member class.
 */

#include "PCH.hpp"
#include "ReflectionMember.hpp"
#include "System/Assertion.hpp"


namespace NFE {
namespace RTTI {


Member::Member(const char* name, size_t offset, const Type* type)
    : mType(type)
    , mName(name)
{
    // validate name
    {
        const size_t len = strlen(name);
        NFE_ASSERT(len > 0, "Member name cannot be empty");

        NFE_ASSERT(isalpha(name[0]), "Invalid member name: '%s'. Must start with a letter", name);

        for (size_t i = 0; i < len; ++i)
        {
            NFE_ASSERT(isalnum(name[i]) || name[i] == '_', "Invalid member name: '%s'", name);
        }
    }

    NFE_ASSERT(offset < std::numeric_limits<uint32>::max(), "Member '%s' offset is too big (%zu bytes)", name, offset);
    mOffset = static_cast<uint32>(offset);  
}


} // namespace RTTI
} // namespace NFE
