/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's Type class.
 */

#include "PCH.hpp"
#include "ReflectionMacros.hpp"

#include "../System/Assertion.hpp"
#include "../Logger/Logger.hpp"
#include "../Config/Config.hpp"

namespace NFE {
namespace RTTI {

namespace {

const char* TypeKindToString(const TypeKind kind)
{
    switch (kind)
    {
    case TypeKind::Fundamental:         return "fundamental";
    case TypeKind::SimpleClass:         return "simple class";
    case TypeKind::PolymorphicClass:    return "polymorphic class";
    case TypeKind::AbstractClass:       return "abstract class";
    }

    return "undefined";
}

} // namespace

void Type::FinishInitialization(TypeInfo&& info)
{
    NFE_ASSERT(!mInitialized, "This type is already initialized");
    NFE_ASSERT(!info.name.empty(), "Type name cannot be empty");
    NFE_ASSERT(info.kind != TypeKind::Undefined, "Type kind cannot be undefined");

    mName = info.name;
    mSize = info.size;
    mAlignment = info.alignment;
    mParent = info.parent;
    mMembers = std::move(info.members);
    mKind = info.kind;
    mSerializationInterface = std::move(info.serializationInterface);

    if (mParent)
    {
        Type* parent = const_cast<Type*>(mParent);
        parent->mChildTypes.push_back(this);
    }

    // TODO print more debug info
    LOG_DEBUG("Registered %s type '%s' (%zu bytes)", TypeKindToString(mKind), mName.c_str(), mSize);

    mInitialized = true;
}

bool Type::IsBaseOf(const Type* baseType, const Type* derivedType)
{
    NFE_ASSERT(baseType->mInitialized, "Base type is not initialized");
    NFE_ASSERT(derivedType->mInitialized, "Derived type is not initialized");

    return (derivedType->mParent == baseType);
}

bool Type::Serialize(const char* objectName, const void* object, Common::Config& config, Common::ConfigObject& configObject) const
{
    NFE_ASSERT(mInitialized, "Type is not initialized");
    NFE_ASSERT(mKind != TypeKind::Undefined, "Invalid type");

    if (mKind == TypeKind::Fundamental)
    {
        NFE_ASSERT(mSerializationInterface, "Fundamental type must have serialization interface defined");

        // serialize with the interface
        return mSerializationInterface->Serialize(objectName, object, config, configObject);
    }

    Common::ConfigObject root;

    // serialize members
    for (const Member& member : mMembers)
    {
        const Type* memberType = member.GetType();
        const char* memberPtr = static_cast<const char*>(object) + member.GetOffset();
        
        if (!memberType->Serialize(member.GetName(), memberPtr, config, root))
        {
            LOG_ERROR("Failed to serialize member '%s'", member.GetName());
            return false;
        }
    }

    config.AddValue(configObject, objectName, root);
    return true;
}

} // namespace RTTI
} // namespace NFE
