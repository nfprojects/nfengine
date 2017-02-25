/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's Type class.
 */

#include "PCH.hpp"
#include "ReflectionClassType.hpp"

#include "../../System/Assertion.hpp"
#include "../../Memory/DefaultAllocator.hpp"
#include "../../Logger/Logger.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;


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
    }

    // sort members by offset in class
    std::sort(mMembers.begin(), mMembers.end(), [](const Member& lhs, const Member& rhs)
    {
        return lhs.GetOffset() < rhs.GetOffset();
    });

    if (mParent)
    {
        // HACK
        ClassType* parent = const_cast<ClassType*>(mParent);
        parent->mChildTypes.push_back(this);
    }

    // print debug info
    // TODO there will be a lot of that stuff, maybe dump to a file?
#ifdef _DEBUG
    {
        if (mParent)
        {
            // TODO parent type may be not initialized yet
            LOG_DEBUG("  - Parent type: %s", mParent->GetName());
        }

        for (const Member& member : mMembers)
        {
            LOG_DEBUG("  - Member '%s': type=%s, offset=%zu", member.GetName(), member.GetType()->GetName(), member.GetOffset());
        }
    }
#endif // _DEBUG
}

bool ClassType::IsA(const Type* baseType) const
{
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

void ClassType::ListSubtypes(std::vector<const ClassType*>& outTypes) const
{
    outTypes.push_back(this);

    for (const ClassType* childType : mChildTypes)
    {
        childType->ListSubtypes(outTypes);
    }
}

size_t ClassType::GetNumOfMembers() const
{
    const size_t parentTypeNumMembers = mParent ? mParent->GetNumOfMembers() : 0;
    return mMembers.size() + parentTypeNumMembers;
}

void ClassType::ListMembers(std::vector<Member>& outMembers) const
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
            LOG_ERROR("Failed to serialize member '%s' in object of type '%s'", member.GetName(), GetName());
            return false;
        }
        config.AddValue(outObject, member.GetName(), memberValue);
    }

    return true;
}

bool ClassType::Serialize(const void* object, Config& config, ConfigValue& outValue) const
{
    ConfigObject root;

    // serialize derived members first
    if (mParent)
    {
        if (!mParent->SerializeDirectly(object, config, root))
        {
            LOG_ERROR("Failed to serialize parent class '%s' of object of type '%s'", mParent->GetName(), GetName());
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

bool ClassType::Deserialize(void* outObject, const Config& config, const ConfigValue& value) const
{
    if (!value.IsObject())
    {
        LOG_ERROR("Expected object value");
        return false;
    }

    // deserialize derived members
    if (mParent)
    {
        if (!mParent->Deserialize(outObject, config, value))
        {
            LOG_ERROR("Failed to deserialize parent class '%s' of object of type '%s'", mParent->GetName(), GetName());
            return false;
        }
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
