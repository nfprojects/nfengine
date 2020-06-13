#pragma once

#include "../nfCommon.hpp"
#include "../Containers/DynArray.hpp"

namespace NFE {
namespace RTTI {

/**
 * A utility class used only for unit test purposes.
 * Allows for marking types and member as missing which emulates the behavior of 
 * deserializing objects that have code-data mismatch.
 */
struct NFCOMMON_API UnitTestHelper
{
    struct MemberTypeRemap
    {
        const Member* member;
        const Type* newType;
    };

    Common::DynArray<const Type*> mMissingTypes;
    Common::DynArray<MemberTypeRemap> mMemberTypeRemappings;
};

} // namespace RTTI
} // namespace NFE
