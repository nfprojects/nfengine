/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's Type class.
 */

#include "PCH.hpp"
#include "ReflectionType.hpp"
#include "../../System/Assertion.hpp"
#include "../../Memory/DefaultAllocator.hpp"
#include "../../Logger/Logger.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"


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
    case TypeKind::DynArray:            return "dynamic array";
    case TypeKind::UniquePtr:           return "unique pointer";
    case TypeKind::SimpleClass:         return "simple class";
    case TypeKind::PolymorphicClass:    return "polymorphic class";
    case TypeKind::AbstractClass:       return "abstract class";
    }

    return "undefined";
}

Type::Type(const TypeInfo& info)
    : mInitialized(false)
{
    if (info.name)
    {
        mName = info.name;
    }

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
    mArrayConstructor = info.arrayConstructor;
}

void Type::PrintInfo() const
{
    LOG_DEBUG("%s (%s): size=%u, alignment=%u", GetName(), TypeKindToString(GetKind()), GetSize(), GetAlignment());
}

bool Type::IsA(const Type* baseType) const
{
    return this == baseType;
}

void* Type::CreateRawObject() const
{
    return mConstructor();
}

} // namespace RTTI
} // namespace NFE
