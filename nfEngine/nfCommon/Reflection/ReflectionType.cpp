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
#include "../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

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

bool Type::Serialize(const void* object, Config& config, ConfigValue& outValue) const
{
    NFE_ASSERT(mInitialized, "Type is not initialized");
    NFE_ASSERT(mKind != TypeKind::Undefined, "Invalid type");
    NFE_ASSERT(object, "Trying to serialize null pointer");

    if (mKind == TypeKind::Fundamental)
    {
        NFE_ASSERT(mSerializationInterface, "Fundamental type must have serialization interface defined");

        // serialize with the interface
        return mSerializationInterface->Serialize(object, outValue);
    }
    else if (mKind == TypeKind::SimpleClass || mKind == TypeKind::PolymorphicClass)
    {
        return SerializeClassType(object, config, outValue);
    }

    LOG_ERROR("Type '%s' cannot be serialized", TypeKindToString(mKind));
    return false;
}

bool Type::SerializeClassType(const void* object, Config& config, ConfigValue& outValue) const
{
    ConfigObject root;

    // TODO serialize parent members

    // serialize members
    for (const Member& member : mMembers)
    {
        const Type* memberType = member.GetType();
        const char* memberPtr = static_cast<const char*>(object) + member.GetOffset();

        if (member.IsArray()) // serialize array member
        {
            const size_t arraySize = member.GetArraySize();
            ConfigArray array;

            // serialize array elements
            for (size_t i = 0; i < arraySize; ++i)
            {
                const char* pointerInArray = memberPtr + i * memberType->GetSize();
                ConfigValue arrayElementValue;
                if (!memberType->Serialize(pointerInArray, config, arrayElementValue))
                {
                    LOG_ERROR("Failed to serialize member '%s'", member.GetName());
                    return false;
                }

                config.AddValue(array, arrayElementValue);
            }

            config.AddValue(root, member.GetName(), array);
        }
        else // serialize scalar member
        {
            ConfigValue memberValue;
            if (!memberType->Serialize(memberPtr, config, memberValue))
            {
                LOG_ERROR("Failed to serialize member '%s'", member.GetName());
                return false;
            }
            config.AddValue(root, member.GetName(), memberValue);
        }
    }

    outValue = ConfigValue(root);
    return true;
}

bool Type::Deserialize(void* outObject, const Config& config, const ConfigValue& value) const
{
    NFE_ASSERT(mInitialized, "Type is not initialized");
    NFE_ASSERT(mKind != TypeKind::Undefined, "Invalid type");
    NFE_ASSERT(outObject, "Trying to deserialize null pointer");

    if (mKind == TypeKind::Fundamental)
    {
        NFE_ASSERT(mSerializationInterface, "Fundamental type must have serialization interface defined");

        if (value.IsArray() || value.IsObject())
        {
            LOG_ERROR("Expected simple type value");
            return false;
        }

        // deserialize with the interface
        return mSerializationInterface->Deserialize(outObject, value);
    }
    else if (mKind == TypeKind::SimpleClass || mKind == TypeKind::PolymorphicClass)
    {
        return DeserializeClassType(outObject, config, value);
    }

    LOG_ERROR("Type '%s' cannot be deserialized", TypeKindToString(mKind));
    return false;
}

bool Type::DeserializeClassType(void* outObject, const Config& config, const ConfigValue& value) const
{
    if (!value.IsObject())
    {
        LOG_ERROR("Expected object value");
        return false;
    }

    bool success = true;
    const auto configIteratorCallback = [outObject, this, &config, &success](const char* key, const ConfigValue& value)
    {
        const Member* targetMember = nullptr;

        // find member with given name
        for (const Member& member : mMembers)
        {
            if (strcmp(member.GetName(), key) == 0)
            {
                targetMember = &member;
                break;
            }
        }

        // target member not found
        if (!targetMember)
        {
            LOG_WARNING("Member '%s' not found in runtime type, but present in deserialized object", key);
            return true;
        }

        // TODO array support

        const Type* memberType = targetMember->GetType();
        char* memberPtr = static_cast<char*>(outObject) + targetMember->GetOffset();

        if (!memberType->Deserialize(memberPtr, config, value))
        {
            LOG_ERROR("Failed to deserialize member '%s'", key);
            success = false;
            return false;
        }

        return true;
    };

    config.Iterate(configIteratorCallback, value.GetObj());
    return success;
}

} // namespace RTTI
} // namespace NFE
