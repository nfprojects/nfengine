/**
 * @file
 * @author Witek902
 * @brief  Definitions of reflection system's ClassType class.
 */

#include "PCH.hpp"
#include "ReflectionClassType.hpp"
#include "ReflectionPointerType.hpp"
#include "../SerializationContext.hpp"
#include "../ReflectionUnitTestHelper.hpp"
#include "../ReflectionTypeRegistry.hpp"
#include "../ReflectionVariant.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

const StringView ClassType::TYPE_MARKER("__type");

using MemberPayloadSizeType = uint16;

ClassType::ClassType()
    : mParent(nullptr)
    , mIsAbstract(false)
{}

void ClassType::OnInitialize(const TypeInfo& info)
{
    const ClassTypeInfo& classTypeInfo = reinterpret_cast<const ClassTypeInfo&>(info);

    mParent = classTypeInfo.parent;
    mIsAbstract = classTypeInfo.isAbstract;

    mMembers.Reserve(classTypeInfo.members.Size());
    for (const Member& member : classTypeInfo.members)
    {
        NFE_ASSERT(nullptr == FindMember(StringView(member.GetName())),
            "Duplicated member '%s' in class '%s'", member.GetName(), GetName().Str());

        mMembers.PushBack(member);

        NFE_ASSERT(member.GetName(), "Member has no name");
        NFE_ASSERT(member.GetType(), "Member '%s' has invalid type", member.GetName());
        NFE_ASSERT(member.GetName() != TYPE_MARKER, "Name reserved");
    }

    std::sort(mMembers.Begin(), mMembers.End(), [] (const Member& a, const Member& b)
    {
        return a.GetOffset() < b.GetOffset();
    });

    if (mParent)
    {
        // HACK
        ClassType* parent = const_cast<ClassType*>(mParent);
        parent->mChildTypes.PushBack(this);
    }

#ifndef NFE_CONFIGURATION_FINAL
    // validate default object
    if (mDefaultObject)
    {
        for (const Member& member : mMembers)
        {
            if (member.GetMetadata().nonNull)
            {
                // check if not null if member has "nonNull" flag on
                if (member.GetType()->GetKind() == TypeKind::SharedPtr || member.GetType()->GetKind() == TypeKind::UniquePtr)
                {
                    const PointerType* pointerType = static_cast<const PointerType*>(member.GetType());
                    NFE_ASSERT(nullptr != pointerType->GetPointedDataType(member.GetMemberPtr(mDefaultObject)),
                        "Property '%s' in class '%s' was marked as non-null, but the default's object propery is nullptr",
                        member.GetName(), this->GetName().Str());
                }
            }
        }
    }
#endif // NFE_CONFIGURATION_FINAL
}

void ClassType::PrintInfo() const
{
    Type::PrintInfo();

#ifdef NFE_CONFIGURATION_DEBUG
    if (mParent)
    {
        NFE_LOG_DEBUG("    - Parent type: %s", mParent->GetName().Str());
    }

    for (const Member& member : mMembers)
    {
        NFE_LOG_DEBUG("    - Member '%s': type=%s, offset=%u", member.GetName(), member.GetType()->GetName().Str(), member.GetOffset());
    }
#endif // NFE_CONFIGURATION_DEBUG
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
    if (!IsAbstract() || !skipAbstractTypes)
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
    // TODO hashmap or binary search

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

void ClassType::CollectDifferingMemberList(const void* objectA, const void* objectB, DynArray<const Member*>& outMemberList) const
{
    if (mParent)
    {
        mParent->CollectDifferingMemberList(objectA, objectB, outMemberList);
    }

    for (const Member& member : mMembers)
    {
        if (member.GetMetadata().NonSerialized())
        {
            continue;
        }

        if (member.GetType()->Compare(member.GetMemberPtr(objectA), member.GetMemberPtr(objectB)))
        {
            continue;
        }

        outMemberList.PushBack(&member);
    }
}

bool ClassType::SerializeDirectly(const void* object, IConfig& config, ConfigObject& outObject, SerializationContext& context) const
{
    // serialize members
    for (const Member& member : mMembers)
    {
        const Type* memberType = member.GetType();

        ConfigValue memberValue;
        if (!memberType->Serialize(member.GetMemberPtr(object), config, memberValue, context))
        {
            NFE_LOG_ERROR("Failed to serialize member '%s' in object of type '%s'", member.GetName(), GetName().Str());
            return false;
        }

        // TODO Member::GetName should contain StringValue instead of const char* ?
        config.AddValue(outObject, StringView(member.GetName()), memberValue);
    }

    return true;
}

bool ClassType::Serialize(const void* object, IConfig& config, ConfigValue& outValue, SerializationContext& context) const
{
    if (IsAbstract())
    {
        NFE_LOG_ERROR("Trying to serialize abstract type '%s'", GetName().Str());
        return false;
    }

    ConfigObject root;

    // attach type marker
    if (IsA(GetType<IObject>()))
    {
        ConfigValue marker(GetName().Str());
        config.AddValue(root, TYPE_MARKER, marker);
    }

    // serialize derived members first
    if (mParent)
    {
        if (!mParent->SerializeDirectly(object, config, root, context))
        {
            NFE_LOG_ERROR("Failed to serialize parent class '%s' of object of type '%s'", mParent->GetName().Str(), GetName().Str());
            return false;
        }
    }

    if (!SerializeDirectly(object, config, root, context))
    {
        return false;
    }

    outValue = ConfigValue(root);
    return true;
}

bool ClassType::DeserializeMember(void* outObject, const StringView memberName, const IConfig& config, const ConfigValue& value, SerializationContext& context) const
{
    // find member with given name
    const Member* targetMember = FindMember(memberName);

    // target member not found
    if (!targetMember)
    {
        // TODO report missing member
        NFE_LOG_WARNING("Member '%.*s' not found in runtime type, but present in deserialized object", memberName.Length(), memberName.Data());
        return true;
    }

    const Type* memberType = targetMember->GetType();

    if (!memberType->Deserialize(targetMember->GetMemberPtr(outObject), config, value, context))
    {
        NFE_LOG_ERROR("Failed to deserialize member '%.*s'", memberName.Length(), memberName.Data());
        return false;
    }

    return true;
}

bool ClassType::Deserialize(void* outObject, const IConfig& config, const ConfigValue& value, SerializationContext& context) const
{
    if (IsAbstract())
    {
        // TODO report type mismatch
        NFE_LOG_ERROR("Trying to deserialize abstract type '%s'", GetName().Str());
        return false;
    }

    if (!value.IsObject())
    {
        // TODO report type mismatch
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
                // TODO report type mismatch
                NFE_LOG_ERROR("Marker type found - string expected");
                success = false;
                return false;
            }

            const char* typeName = value.Get<const char*>();
            if (typeName != GetName())
            {
                // TODO report type mismatch
                NFE_LOG_ERROR("Invalid polymorphic type in marker: found '%s', expected '%s'", typeName, GetName().Str());
                success = false;
                return false;
            }

            return true;
        }

        if (!DeserializeMember(outObject, key, config, value, context))
        {
            success = false;
            return false;
        }

        return true;
    };

    config.Iterate(configIteratorCallback, value.GetObj());
    return success;
}

bool ClassType::SerializeBinary(const void* object, OutputStream* stream, SerializationContext& context) const
{
    NFE_ASSERT(object, "Invalid object ptr");
    NFE_ASSERT(!IsAbstract(), "Tring to serialize abstract class");

    // collect list of members to serialize
    // TODO get rid of dynamic allocation
    Common::DynArray<const Member*> membersToSerialize;
    CollectDifferingMemberList(object, mDefaultObject, membersToSerialize);

    TypeRegistry& typeRegistry = TypeRegistry::GetInstance();

    if (!context.IsMapping())
    {
        if (!stream->WriteCompressedUint(membersToSerialize.Size()))
        {
            return false;
        }
    }

    for (const Member* member : membersToSerialize)
    {
        const Type* memberType = member->GetType();

        // TODO instead of storing type names as strings use some compressed (symbolic) form?

        const uint32 memberNameStrIndex = context.MapString(member->GetName());

        uint64 streamPosBeforeObject = UINT64_MAX;

        if (!typeRegistry.SerializeTypeName(memberType, stream, context))
        {
            return false;
        }

        if (!context.IsMapping())
        {
            // write member name
            if (!stream->WriteCompressedUint(memberNameStrIndex))
            {
                return false;
            }

            streamPosBeforeObject = stream->GetPosition();

            MemberPayloadSizeType memberPayloadSize = 0;
            if (!stream->Write(memberPayloadSize))
            {
                return false;
            }
        }

        // deserialize the member if the type is correct
        if (!memberType->SerializeBinary(member->GetMemberPtr(object), stream, context))
        {
            return false;
        }

        // write member payload size
        if (!context.IsMapping())
        {
            const uint64 streamPosAfterObject = stream->GetPosition();

            // Note: subtract sizeof(MemberPayloadSizeType), because (streamPosAfterObject-streamPosBeforeObject) includes
            // both the object and payload size marker in the data stream
            const uint64 payloadSize = streamPosAfterObject - streamPosBeforeObject - sizeof(MemberPayloadSizeType);
            if (payloadSize > std::numeric_limits<MemberPayloadSizeType>::max())
            {
                NFE_LOG_ERROR("Sserialization failed. Member payload is too large");
                return false;
            }

            if (!stream->Seek(streamPosBeforeObject, SeekMode::Begin))
            {
                NFE_LOG_ERROR("Sserialization failed. Failed to seek to write member payload size");
                return false;
            }

            const MemberPayloadSizeType payloadSizeToWrite = static_cast<MemberPayloadSizeType>(payloadSize);
            if (!stream->Write(payloadSizeToWrite))
            {
                NFE_LOG_ERROR("Sserialization failed. Failed to write member payload size");
                return false;
            }

            if (!stream->Seek(streamPosAfterObject, SeekMode::Begin))
            {
                NFE_LOG_ERROR("Sserialization failed. Failed to seek back after writing member payload size");
                return false;
            }
        }

        // TODO update payload size
    }

    return true;
}

bool ClassType::DeserializeBinary(void* outObject, InputStream& stream, SerializationContext& context) const
{
    uint32 numMembers;
    if (!stream.ReadCompressedUint(numMembers))
    {
        return false;
    }

    // TODO bulk deserialization of POD types

    TypeRegistry& typeRegistry = TypeRegistry::GetInstance();

    const UnitTestHelper* unitTestHelper = context.GetUnitTestHelper();

    // patch serialized members only
    for (uint32 i = 0; i < numMembers; ++i)
    {
        // deserialize type from stream
        const Type* serializedType = nullptr;
        if (TypeDeserializationResult::Error == typeRegistry.DeserializeTypeName(serializedType, stream, context))
        {
            NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
            return false;
        }

        // read member name
        StringView memberName;
        {
            uint32 strIndex;
            if (!stream.ReadCompressedUint(strIndex))
            {
                NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
                return false;
            }
            if (!context.UnmapString(strIndex, memberName))
            {
                NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
                return false;
            }
        }

        // read amount of bytes in the stream for this member
        // note: this value will be positive
        MemberPayloadSizeType memberPayloadSize = 0;
        if (!stream.Read(memberPayloadSize))
        {
            NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
            return false;
        }

        if (unitTestHelper)
        {
            if (unitTestHelper->mMissingTypes.Find(serializedType) != unitTestHelper->mMissingTypes.End())
            {
                serializedType = nullptr;
            }
        }

        // serialized type not known
        if (!serializedType)
        {
            // TODO report unknown type
            NFE_LOG_WARNING("Failed to deserialize member '%.*s' of class %s. Type in data is not know. Probably the type was removed from code.",
                memberName.Length(), memberName.Data(), GetName().Str());

            if (!stream.Seek(memberPayloadSize, SeekMode::Current))
            {
                NFE_LOG_ERROR("Deserialization failed. Failed to skip %u bytes. Corrupted data?", (uint32)memberPayloadSize);
                return false;
            }
            continue;
        }

        // find member with given name
        const Member* targetMember = FindMember(memberName);

        // target member not found
        if (!targetMember)
        {
            NFE_LOG_WARNING("Failed to deserialize member '%.*s' - it's not present in class %s. Probably it was removed from code.",
                memberName.Length(), memberName.Data(), GetName().Str());

            Buffer tempObjectData;
            if (!tempObjectData.Resize(serializedType->GetSize(), nullptr, serializedType->GetAlignment()))
            {
                NFE_LOG_ERROR("Deserialization failed. Failed to allocate buffer for temporary object");
                return false;
            }

            // deserialize to a temporary object
            serializedType->ConstructObject(tempObjectData.Data());
            if (!serializedType->DeserializeBinary(tempObjectData.Data(), stream, context))
            {
                return false;
            }

            Variant readObject(serializedType, std::move(tempObjectData));

            // report missing memeber, so the type can be upgraded manually
            MemberPath memberPath(memberName);
            context.PushMemberTypeMismatchInfo(memberPath, std::move(readObject));

            continue;
        }

        const Type* memberType = targetMember->GetType();

        // member found and type know, but it does not match the type used in code
        if (memberType != serializedType)
        {
            Buffer tempObjectData;
            if (!tempObjectData.Resize(serializedType->GetSize(), nullptr, serializedType->GetAlignment()))
            {
                NFE_LOG_ERROR("Deserialization failed. Failed to allocate buffer for temporary object");
                return false;
            }

            // deserialize to a temporary object
            serializedType->ConstructObject(tempObjectData.Data());
            if (!serializedType->DeserializeBinary(tempObjectData.Data(), stream, context))
            {
                return false;
            }

            Variant readObject(serializedType, std::move(tempObjectData));

            if (memberType->TryLoadFromDifferentType(targetMember->GetMemberPtr(outObject), readObject))
            {
                NFE_LOG_DEBUG("Successfully upgraded member '%.*s' of class %s. Type in data was '%s', but in code it is '%s'",
                    memberName.Length(), memberName.Data(), GetName().Str(), serializedType->GetName().Str(), memberType->GetName().Str());
                continue;
            }

            NFE_LOG_WARNING("Failed to deserialize member '%.*s' of class %s. Type in data is '%s', it's expected to be '%s'",
                memberName.Length(), memberName.Data(), GetName().Str(), serializedType->GetName().Str(), memberType->GetName().Str());

            // report type mismatch, so the type can be upgraded manually
            MemberPath memberPath(targetMember->GetName());
            context.PushMemberTypeMismatchInfo(memberPath, std::move(readObject));

            continue;
        }

        // deserialize the member if the type is correct
        if (!memberType->DeserializeBinary(targetMember->GetMemberPtr(outObject), stream, context))
        {
            return false;
        }
    }

    return true;
}

bool ClassType::SerializeTypeName(Common::OutputStream* stream, SerializationContext& context) const
{
    // write header
    if (!Type::SerializeTypeName(stream, context))
    {
        return false;
    }

    // append class name
    const uint32 typeNameStrIndex = context.MapString(GetName());
    if (!context.IsMapping())
    {
        if (!stream->WriteCompressedUint(typeNameStrIndex))
        {
            return false;
        }
    }

    return true;
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
        if (!member.GetType()->Compare(member.GetMemberPtr(objectA), member.GetMemberPtr(objectB)))
        {
            return false;
        }
    }

    return true;
}


bool ClassType::Clone(void* destObject, const void* sourceObject) const
{
    bool success = true;

    if (mParent)
    {
        success &= mParent->Clone(destObject, sourceObject);
    }

    // clone members
    for (const Member& member : mMembers)
    {
        const Type* memberType = member.GetType();
        success &= memberType->Clone(member.GetMemberPtr(destObject), member.GetMemberPtr(sourceObject));
    }

    return success;
}

} // namespace RTTI
} // namespace NFE
