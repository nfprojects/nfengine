/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of reflection system's Member class.
 */

#pragma once

#include "PCH.hpp"
#include "ReflectionMember.hpp"
#include "System/Assertion.hpp"


namespace NFE {
namespace RTTI {


Member::Member(const char* name, size_t offset, const Type* type)
    : mName(name)
    , mType(type)
{
    NFE_ASSERT(offset < std::numeric_limits<uint32>::max(), "Member offset is too big");
    mOffset = static_cast<uint32>(offset);
}


} // namespace RTTI
} // namespace NFE
