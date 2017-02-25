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

Type::Type(const char* name)
    : mName(name)
    , mSize(0)
    , mAlignment(0)
    , mParent(nullptr)
    , mKind(TypeKind::Undefined)

{
}

void Type::FinishInitialization(TypeInfo&& info)
{
    NFE_ASSERT(!mInitialized, "This type is already initialized");
    NFE_ASSERT(info.name, "Type name cannot be empty");
    NFE_ASSERT(info.kind != TypeKind::Undefined, "Type kind cannot be undefined");

    // downcast to 4 bytes to save space
    NFE_ASSERT(info.size < UINT32_MAX, "Type size is too big");
    NFE_ASSERT(info.alignment < UINT32_MAX, "Type size is too big");

    mSize = static_cast<uint32>(info.size);
    mAlignment = static_cast<uint32>(info.alignment);
    mParent = info.parent;
    mMembers = std::move(info.members);
    mKind = info.kind;
    mSerializer = std::move(info.serializer);

    // verify members
    for (const Member& member : mMembers)
    {
        NFE_ASSERT(member.GetName(), "Member has no name");
        NFE_ASSERT(member.GetType(), "Member '%s' has invalid type", member.GetName());
    }

    // sort members by offset in class
    std::sort(mMembers.begin(), mMembers.end(), [](const Member& lhs, const Member& rhs)
    {
        return lhs.GetOffset() < rhs.GetOffset();
    });

    if (mParent)
    {
        // HACK
        Type* parent = const_cast<Type*>(mParent);
        parent->mChildTypes.push_back(this);
    }

    // print debug info
    // TODO there will be a lot of that stuff, maybe dump to a file?
    {
        LOG_DEBUG("Registered %s type '%s' (%u bytes)", TypeKindToString(mKind), GetName(), mSize);

        if (mParent)
        {
            // TODO parent type may be not initialized yet
            LOG_DEBUG("    Parent type: %s", mParent->GetName());
        }

        for (const Member& member : mMembers)
        {
            LOG_DEBUG("    Member '%s': type=%s, offset=%zu, arraySize=%zu",
                      member.GetName(), member.GetType()->GetName(), member.GetOffset(), member.GetArraySize());
        }
    }

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
        NFE_ASSERT(mSerializer, "Fundamental type must have serialization interface defined");

        // serialize with the interface
        return mSerializer->Serialize(object, config, outValue);
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
        NFE_ASSERT(mSerializer, "Fundamental type must have serialization interface defined");

        if (value.IsArray() || value.IsObject())
        {
            LOG_ERROR("Expected simple type value");
            return false;
        }

        // deserialize with the interface
        return mSerializer->Deserialize(outObject, config, value);
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
    const auto configIteratorCallback = [&](const char* key, const ConfigValue& value)
    {
        // find member with given name
        const Member* targetMember = nullptr;
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

        const Type* memberType = targetMember->GetType();
        char* memberPtr = static_cast<char*>(outObject) + targetMember->GetOffset();

        if (targetMember->IsArray()) // deserialize array
        {
            if (!value.IsArray())
            {
                LOG_ERROR("Expected array type for key '%s'", key);
                success = false;
                return false;
            }

            size_t numDeserializedArrayElements = 0;
            auto arrayIteratorCallback = [&](int index, const ConfigValue& arrayElement)
            {
                if (index >= targetMember->GetArraySize())
                {
                    LOG_WARNING("Deserialized member '%s' has too many array elements (%zu expected). "
                                "Target elements will be dropped", targetMember->GetName(), targetMember->GetArraySize());
                    return false;
                }

                char* pointerInArray = memberPtr + index * memberType->GetSize();
                if (!memberType->Deserialize(pointerInArray, config, arrayElement))
                {
                    LOG_ERROR("Failed to member '%s' at index %i", targetMember->GetName(), index);
                    success = false;
                    return false;
                }

                numDeserializedArrayElements++;
                return true;
            };

            config.IterateArray(arrayIteratorCallback, value.GetArray());

            if (numDeserializedArrayElements != targetMember->GetArraySize())
            {
                LOG_WARNING("Deserialized member '%s' has too few array elements (%zu found, %zu expected).",
                            targetMember->GetName(), numDeserializedArrayElements, targetMember->GetArraySize());
                // TODO initialize missing elements with default values (run default constructor)
            }
        }
        else // deserialize scalar
        {
            if (!memberType->Deserialize(memberPtr, config, value))
            {
                LOG_ERROR("Failed to deserialize member '%s'", key);
                success = false;
                return false;
            }
        }

        return true;
    };

    config.Iterate(configIteratorCallback, value.GetObj());
    return success;
}

} // namespace RTTI
} // namespace NFE
