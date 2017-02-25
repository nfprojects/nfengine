/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's Type class.
 */

#include "PCH.hpp"
#include "ReflectionMacros.hpp"
#include "../System/Assertion.hpp"
#include "../Logger/Logger.hpp"


namespace NFE {
namespace RTTI {

void Type::FinishInitialization(const TypeInfo& info)
{
    NFE_ASSERT(!mInitialized, "This type is already initialized");
    NFE_ASSERT(!mName.empty(), "Type name cannot be empty");

    mName = info.name;
    mSize = info.size;
    mAlignment = info.alignment;
    mParent = info.parent;
    mMembers = info.members;

    // TODO print more debug info
    LOG_DEBUG("Registered type '%s' (%zu bytes)", mName.c_str(), mSize);

    mInitialized = true;
}

bool Type::DefineInheritance(const Type* parent, const Type* child)
{
    //NFE_ASSERT(parent->mInitialized, "Parent class is not initialized");

    if (child->mParent)
    {
        // TODO multiple inheritance is not supported yet
        return false;
    }

    // nice hack
    Type* parentType = const_cast<Type*>(parent);
    Type* childType = const_cast<Type*>(child);

    // TODO this could be done in some post-processing step
    parentType->mChildTypes.push_back(child);

    childType->mParent = parent;
    return true;
}

bool Type::IsDerivedFrom(const Type* type) const
{
    NFE_ASSERT(type->mInitialized, "Type is not initialized");

    // TODO
    UNUSED(type);
    return false;
}

} // namespace RTTI
} // namespace NFE
