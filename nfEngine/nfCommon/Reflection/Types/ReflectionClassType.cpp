/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's ClassType class.
 */

#include "PCH.hpp"
#include "ReflectionClassType.hpp"

#include "../../Config/Config.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

const StringView ClassType::TYPE_MARKER("__type");

ClassType::ClassType(const ClassTypeInfo& info)
    : Type(info)
    , mParent(info.parent)
    , mMembers(info.members)
{
    // verify members
    for (const Member& member : mMembers)
    {
        NFE_ASSERT(member.GetName(), "Member has no name");
        NFE_ASSERT(member.GetType(), "Member '%s' has invalid type", member.GetName());
        NFE_ASSERT(member.GetName() != TYPE_MARKER, "Name reserved");
    }

    if (mParent)
    {
        // HACK
        ClassType* parent = const_cast<ClassType*>(mParent);
        parent->mChildTypes.PushBack(this);
    }
}

void ClassType::PrintInfo() const
{
    Type::PrintInfo();

#ifdef _DEBUG
    {
        if (mParent)
        {
            NFE_LOG_DEBUG("    - Parent type: %s", mParent->GetName());
        }

        for (const Member& member : mMembers)
        {
            NFE_LOG_DEBUG("    - Member '%s': type=%s, offset=%u", member.GetName(), member.GetType()->GetName(), member.GetOffset());
        }
    }
#endif // _DEBUG
}

bool ClassType::IsA(const Type* baseType) const
{
    if (this == baseType)
    {
        return true;
    }

    const ClassType* parentType = mParent;
    while (parentType)
    {
        if (parentType == baseType)
        {
            return true;
        }

        parentType = parentType->mParent;
    }

    return false;
}

void ClassType::ListSubtypes(Children& outTypes, bool skipAbstractTypes) const
{
    ListSubtypes([&outTypes] (const ClassType* type)
    {
        outTypes.PushBack(type);
    }, skipAbstractTypes);
}

void ClassType::ListSubtypes(const std::function<void(const ClassType*)>& func, bool skipAbstractTypes) const
{
    if (GetKind() != TypeKind::AbstractClass || !skipAbstractTypes)
    {
        func(this);
    }

    for (const ClassType* childType : mChildTypes)
    {
        childType->ListSubtypes(func, skipAbstractTypes);
    }
}

size_t ClassType::GetNumOfMembers() const
{
    const size_t parentTypeNumMembers = mParent ? mParent->GetNumOfMembers() : 0;
    return mMembers.Size() + parentTypeNumMembers;
}

void ClassType::ListMembers(Members& outMembers) const
{
    if (mParent)
    {
        mParent->ListMembers(outMembers);
    }

    for (const Member& member : mMembers)
    {
        outMembers.PushBack(member);
    }
}

const Member* ClassType::FindMember(const StringView name) const
{
    for (const Member& member : mMembers)
    {
        if (name == StringView(member.GetName()))
        {
            return &member;
        }
    }

    if (mParent)
    {
        return mParent->FindMember(name);
    }

    return nullptr;
}

bool ClassType::SerializeDirectly(const void* object, Common::Config& config, Common::ConfigObject& outObject) const
{
    // serialize members
    for (const Member& member : mMembers)
    {
        const Type* memberType = member.GetType();
        const char* memberPtr = static_cast<const char*>(object) + member.GetOffset();

        ConfigValue memberValue;
        if (!memberType->Serialize(memberPtr, config, memberValue))
        {
            NFE_LOG_ERROR("Failed to serialize member '%s' in object of type '%s'", member.GetName(), GetName());
            return false;
        }

        // TODO Member::GetName should contain StringValu instead of const char* ?
        config.AddValue(outObject, StringView(member.GetName()), memberValue);
    }

    return true;
}

bool ClassType::Serialize(const void* object, Config& config, ConfigValue& outValue) const
{
    if (GetKind() == TypeKind::AbstractClass)
    {
        NFE_LOG_ERROR("Trying to serialize abstract type '%s'", GetName());
        return false;
    }

    ConfigObject root;

    // attach type marker
    if (GetKind() == TypeKind::PolymorphicClass)
    {
        ConfigValue marker(GetName());
        config.AddValue(root, TYPE_MARKER, marker);
    }

    // serialize derived members first
    if (mParent)
    {
        if (!mParent->SerializeDirectly(object, config, root))
        {
            NFE_LOG_ERROR("Failed to serialize parent class '%s' of object of type '%s'", mParent->GetName(), GetName());
            return false;
        }
    }

    if (!SerializeDirectly(object, config, root))
    {
        return false;
    }

    outValue = ConfigValue(root);
    return true;
}

bool ClassType::DeserializeMember(void* outObject, const StringView memberName, const Config& config, const ConfigValue& value) const
{
    // find member with given name
    const Member* targetMember = FindMember(memberName);

    // target member not found
    if (!targetMember)
    {
        NFE_LOG_WARNING("Member '%.*s' not found in runtime type, but present in deserialized object",
                        memberName.Length(), memberName.Data());
        return true;
    }

    const Type* memberType = targetMember->GetType();
    char* memberPtr = static_cast<char*>(outObject) + targetMember->GetOffset();

    if (!memberType->Deserialize(memberPtr, config, value))
    {
        NFE_LOG_ERROR("Failed to deserialize member '%.*s'", memberName.Length(), memberName.Data());
        return false;
    }

    return true;
}

bool ClassType::Deserialize(void* outObject, const Config& config, const ConfigValue& value) const
{
    if (GetKind() == TypeKind::AbstractClass)
    {
        NFE_LOG_ERROR("Trying to deserialize abstract type '%s'", GetName());
        return false;
    }

    if (!value.IsObject())
    {
        NFE_LOG_ERROR("Expected object value");
        return false;
    }

    bool success = true;
    const auto configIteratorCallback = [&](StringView key, const ConfigValue& value)
    {
        // polymorphic type marker found
        if (key == TYPE_MARKER)
        {
            if (!value.IsString())
            {
                NFE_LOG_ERROR("Marker type found - string expected");
                success = false;
                return false;
            }

            const char* typeName = value.Get<const char*>();
            if (strcmp(typeName, GetName()) != 0)
            {
                NFE_LOG_ERROR("Invalid polymorphic type in marker: found '%s', expected '%s'", typeName, GetName());
                success = false;
                return false;
            }

            return true;
        }

        if (!DeserializeMember(outObject, key, config, value))
        {
            success = false;
            return false;
        }

        return true;
    };

    config.Iterate(configIteratorCallback, value.GetObj());
    return success;
}

bool ClassType::Compare(const void* objectA, const void* objectB) const
{
    if (mParent)
    {
        if (!mParent->Compare(objectA, objectB))
        {
            return false;
        }
    }

    // compare members
    for (const Member& member : mMembers)
    {
        const Type* memberType = member.GetType();
        const char* memberPtrA = static_cast<const char*>(objectA) + member.GetOffset();
        const char* memberPtrB = static_cast<const char*>(objectB) + member.GetOffset();

        if (!memberType->Compare(memberPtrA, memberPtrB))
        {
            return false;
        }
    }

    return true;
}

} // namespace RTTI
} // namespace NFE
