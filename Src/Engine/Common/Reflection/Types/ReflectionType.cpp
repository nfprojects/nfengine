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


namespace NFE {
namespace RTTI {

using namespace Common;


const char* Type::TypeKindToString(const TypeKind kind)
{
    switch (kind)
    {
    case TypeKind::Fundamental:         return "fundamental";
    case TypeKind::Enumeration:         return "enum";
    case TypeKind::NativeArray:         return "native array";
    case TypeKind::String:              return "string";
    case TypeKind::DynArray:            return "dynamic array";
    case TypeKind::UniquePtr:           return "unique pointer";
    case TypeKind::SharedPtr:           return "shared pointer";
    case TypeKind::SimpleClass:         return "simple class";
    case TypeKind::PolymorphicClass:    return "polymorphic class";
    case TypeKind::AbstractClass:       return "abstract class";
    }

    return "undefined";
}

Type::~Type() = default;

Type::Type(const TypeInfo& info)
    : mDefaultObject(nullptr)
{
    NFE_ASSERT(info.name, "Invalid type name (nullptr)");
    mName = StringView(info.name);
    NFE_ASSERT(!mName.Empty(), "Invalid type name (empty string)");

    NFE_ASSERT(info.kind != TypeKind::Undefined, "Type kind cannot be undefined");
    mKind = info.kind;

    // downcast to 4 bytes to save space - there shouldn't be classes greater than 4GB...
    NFE_ASSERT(info.size < UINT32_MAX, "Type size is too big");
    NFE_ASSERT(info.alignment < UINT32_MAX, "Type alignment is too big");

    NFE_ASSERT(info.size > 0, "Type size cannot be zero");
    NFE_ASSERT(info.alignment > 0, "Type alignment cannot be zero");

    mSize = static_cast<uint32>(info.size);
    mAlignment = static_cast<uint32>(info.alignment);

    mConstructor = info.constructor;
    mDestructor = info.destructor;

    if (mConstructor)
    {
        NFE_ASSERT(mDestructor, "When constructor is provided, destrtuctor must be defined as well");

        mDefaultObject = CreateRawObject();
        NFE_ASSERT(mDefaultObject, "Failed to create default object for type %s", GetName().Str());
    }
}

void Type::PrintInfo() const
{
    NFE_LOG_DEBUG("%s (%s): size=%u, alignment=%u", GetName().Str(), TypeKindToString(GetKind()), GetSize(), GetAlignment());
}

bool Type::IsA(const Type* baseType) const
{
    return this == baseType;
}

void* Type::CreateRawObject() const
{
    NFE_ASSERT(mConstructor, "Cannot create an object of type '%s'", GetName().Str());
    return mConstructor();
}

void Type::DeleteObject(void* objectPtr) const
{
    NFE_ASSERT(mDestructor, "Cannot destroy an object of type '%s'", GetName().Str());
    mDestructor(objectPtr);
}

bool Type::CanBeMemcopied() const
{
    return false;
}

} // namespace RTTI
} // namespace NFE
