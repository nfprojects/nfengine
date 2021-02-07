/**
 * @file
 * @author Witek902
 * @brief  Definitions of reflection system's Type class.
 */

#include "PCH.hpp"
#include "ReflectionType.hpp"
#include "../../Logger/Logger.hpp"
#include "../../Containers/StringView.hpp"
#include "../../Memory/DefaultAllocator.hpp"
#include "../../Utils/Stream/OutputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;


const char* Type::TypeKindToString(const TypeKind kind)
{
    switch (kind)
    {
    case TypeKind::Fundamental:         return "fundamental";
    case TypeKind::Enumeration:         return "enum";
    case TypeKind::String:              return "string";
    case TypeKind::NativeArray:         return "native array";
    case TypeKind::Array:               return "array";
    case TypeKind::UniquePtr:           return "unique pointer";
    case TypeKind::SharedPtr:           return "shared pointer";
    case TypeKind::Class:               return "class";
    }

    NFE_FATAL("Invalid RTTI type kind: %u", (uint32)kind);
    return "undefined";
}

Type::~Type() = default;

Type::Type()
    : mSize(0u)
    , mAlignment(0u)
    , mKind(TypeKind::Undefined)
    , mDefaultObject(nullptr)
    , mIsDynamicType(false)
    , mIsEnabled(true)
{
}

void Type::Initialize(const TypeInfo& info)
{
    NFE_ASSERT(info.name, "Invalid type name (nullptr)");
    mName = StringView(info.name);
    NFE_ASSERT(!mName.Empty(), "Invalid type name (empty string)");

    NFE_ASSERT(info.kind != TypeKind::Undefined, "Type kind cannot be undefined");
    mKind = info.kind;

    NFE_ASSERT(info.typeNameID != TypeNameID::Invalid, "Invalid type name ID");
    mTypeNameID = info.typeNameID;

    // downcast to 4 bytes to save space - there shouldn't be classes greater than 4GB...
    NFE_ASSERT(info.size < UINT32_MAX, "Type size is too big");
    NFE_ASSERT(info.alignment < UINT32_MAX, "Type alignment is too big");

    NFE_ASSERT(info.size > 0, "Type size cannot be zero");
    NFE_ASSERT(info.alignment > 0, "Type alignment cannot be zero");

    mSize = static_cast<uint32>(info.size);
    mAlignment = static_cast<uint32>(info.alignment);
    mIsDynamicType = info.dynamicType;

    mConstructor = info.constructor;
    mDestructor = info.destructor;

    if (mConstructor)
    {
        mDefaultObject = CreateRawObject();
        NFE_ASSERT(mDefaultObject, "Failed to create default object for type %s", GetName().Str());
    }

    OnInitialize(info);
}

void Type::OnInitialize(const TypeInfo& info)
{
    NFE_UNUSED(info);
}

void Type::PrintInfo() const
{
    NFE_LOG_DEBUG("%s (%s): size=%u, alignment=%u, dynamic=%s",
        GetName().Str(), TypeKindToString(GetKind()), GetSize(), GetAlignment(), mIsDynamicType ? "true" : "false");
}

bool Type::IsA(const Type* baseType) const
{
    return this == baseType;
}

void Type::ConstructObject(void* objectPtr) const
{
    NFE_ASSERT(!mIsDynamicType, "Dynamic type object should be used only for deserialization");
    NFE_ASSERT(mConstructor, "Cannot create an object of type '%s'", GetName().Str());

    mConstructor(objectPtr);
}

void Type::DestructObject(void* objectPtr) const
{
    NFE_ASSERT(!mIsDynamicType, "Dynamic type object should be used only for deserialization");

    if (mDestructor)
    {
        mDestructor(objectPtr);
    }
}

void* Type::CreateRawObject() const
{
    NFE_ASSERT(!mIsDynamicType, "Dynamic type object should be used only for deserialization");
    NFE_ASSERT(mConstructor, "Cannot create an object of type '%s'", GetName().Str());

    void* objectMemory = NFE_MALLOC(mSize, mAlignment);
    if (objectMemory)
    {
        ConstructObject(objectMemory);
    }

    return objectMemory;
}

void Type::DeleteObject(void* objectPtr) const
{
    NFE_ASSERT(!mIsDynamicType, "Dynamic type object should be used only for deserialization");
    NFE_ASSERT(mDestructor, "Cannot destroy an object of type '%s'", GetName().Str());
    
    if (objectPtr)
    {
        DestructObject(objectPtr);
        NFE_FREE(objectPtr);
    }
}

bool Type::TryLoadFromDifferentType(void* outObject, const Variant& otherObject) const
{
    NFE_UNUSED(outObject);
    NFE_UNUSED(otherObject);
    return false;
}

bool Type::CanBeMemcopied() const
{
    return false;
}

bool Type::SerializeTypeName(Common::OutputStream* stream, SerializationContext& context) const
{
    NFE_ASSERT(!mIsDynamicType, "Dynamic type object should be used only for deserialization");

    NFE_UNUSED(context);

    uint8 typeNameId = static_cast<uint8>(mTypeNameID);
    if (stream)
    {
        return stream->Write(typeNameId);
    }

    return true;
}

} // namespace RTTI
} // namespace NFE
