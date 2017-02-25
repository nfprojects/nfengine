/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's type registry.
 */


#include "PCH.hpp"
#include "ReflectionTypeRegistry.hpp"
#include "Types/ReflectionType.hpp"

#include "../Logger/Logger.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace RTTI {


namespace {

const char* TypeKindToString(const TypeKind kind)
{
    switch (kind)
    {
    case TypeKind::Fundamental:         return "fundamental";
    case TypeKind::NativeArray:         return "native array";
    case TypeKind::DynArray:            return "dynamic array";
    case TypeKind::UniquePtr:           return "unique pointer";
    case TypeKind::SimpleClass:         return "simple class";
    case TypeKind::PolymorphicClass:    return "polymorphic class";
    case TypeKind::AbstractClass:       return "abstract class";
    }

    return "undefined";
}

} // namespace


TypeRegistry& TypeRegistry::GetInstance()
{
    static TypeRegistry registry;
    return registry;
}

const Type* TypeRegistry::GetExistingType(size_t hash) const
{
    const auto iter = mTypesMap.find(hash);
    if (iter == mTypesMap.end())
    {
        return nullptr;
    }

    return iter->second.get();
}

const Type* TypeRegistry::RegisterType(size_t hash, TypePtr&& type)
{
    NFE_ASSERT(type, "Invalid type pointer");

    const auto iter = mTypesMap.find(hash);
    NFE_ASSERT(type, "Type with given hash already exists (%s)", iter->second->GetName());

    const Type* typePtr = type.get();
    mTypesMap[hash] = std::move(type);

    LOG_DEBUG("Registered %s type '%s': size=%u, alignment=%u",
              TypeKindToString(typePtr->GetKind()), typePtr->GetName(), typePtr->GetSize(), typePtr->GetAlignment());

    return typePtr;
}


} // namespace RTTI
} // namespace NFE
