/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's Type class.
 */

#include "PCH.hpp"
#include "ReflectionMacros.hpp"

#include "../System/Assertion.hpp"
#include "../Memory/DefaultAllocator.hpp"
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
    NFE_ASSERT(info.name, "Type name cannot be empty");
    NFE_ASSERT(info.kind != TypeKind::Undefined, "Type kind cannot be undefined");

    // downcast to 4 bytes to save some space
    NFE_ASSERT(info.size < UINT32_MAX, "Type size is too big");
    NFE_ASSERT(info.alignment < UINT32_MAX, "Type size is too big");
    mSize = static_cast<uint32>(info.size);
    mAlignment = static_cast<uint32>(info.alignment);

    mName = info.name;
    mParent = info.parent;
    mMembers = std::move(info.members);
    mKind = info.kind;
    mSerializationInterface = std::move(info.serializationInterface);

    // sort members by offset in class
    std::sort(mMembers.begin(), mMembers.end(), [](const Member& lhs, const Member& rhs)
    {
        return lhs.GetOffset() < rhs.GetOffset();
    });

    if (mParent)
    {
        Type* parent = const_cast<Type*>(mParent);
        parent->mChildTypes.push_back(this);
    }

    // TODO print more debug info
    LOG_DEBUG("Registered %s type '%s' (%u bytes)", TypeKindToString(mKind), mName, mSize);

    mInitialized = true;
}

bool Type::IsBaseOf(const Type* baseType, const Type* derivedType)
{
    NFE_ASSERT(baseType->mInitialized, "Base type is not initialized");
    NFE_ASSERT(derivedType->mInitialized, "Derived type is not initialized");

    return (derivedType->mParent == baseType);
}

void Type::ListSubtypes(std::vector<const Type*>& outTypes) const
{
    outTypes.push_back(this);

    for (const Type* childType : mChildTypes)
    {
        childType->ListSubtypes(outTypes);
    }
}

size_t Type::GetNumOfMembers() const
{
    const size_t parentTypeNumMembers = mParent ? mParent->GetNumOfMembers() : 0;
    return mMembers.size() + parentTypeNumMembers;
}

void Type::ListMembers(std::vector<Member>& outMembers) const
{
    if (mParent)
    {
        mParent->ListMembers(outMembers);
    }

    for (const Member& member : mMembers)
    {
        outMembers.push_back(member);
    }
}

void* Type::CreateObjectRaw() const
{
    void* object = NFE_MALLOC(mSize, mAlignment);
    if (!object)
    {
        return nullptr;
    }

    // TODO call the constructor

    return object;
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
    // TODO include parent type members
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

bool Type::Deserialize(void* object, const Common::Config& config, const Common::ConfigObject& configObject) const
{
    NFE_ASSERT(mInitialized, "Type is not initialized");
    NFE_ASSERT(mKind != TypeKind::Undefined, "Invalid type");

    if (mKind == TypeKind::Fundamental)
    {
        NFE_ASSERT(mSerializationInterface, "Fundamental type must have serialization interface defined");

        // deserialize with the interface
        return mSerializationInterface->Deserialize(object, config, configObject);
    }

    // TODO iterate config value
    // TODO include parent type members

    return false;
}

} // namespace RTTI
} // namespace NFE
