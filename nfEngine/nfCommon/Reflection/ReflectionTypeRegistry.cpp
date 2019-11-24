/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's type registry.
 */

#include "PCH.hpp"
#include "ReflectionTypeRegistry.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

ITypeRegistry& ITypeRegistry::GetInstance()
{
    return TypeRegistry::GetInstance();
}

TypeRegistry& TypeRegistry::GetInstance()
{
    static TypeRegistry registry;
    return registry;
}

const Type* TypeRegistry::GetExistingType(size_t hash) const
{
    const auto iter = mTypesByHash.Find(hash);

    if (iter == mTypesByHash.End())
    {
        return nullptr;
    }

    return iter->second;
}

const Type* TypeRegistry::GetExistingType(const char* name) const
{
    return GetExistingType(StringView(name));
}

const Type* TypeRegistry::GetExistingType(const StringView name) const
{
    const auto iter = mTypesByName.Find(name);

    if (iter == mTypesByName.End())
    {
        return nullptr;
    }

    return iter->second;
}

const Type* TypeRegistry::RegisterType(size_t hash, Type* type)
{
    NFE_ASSERT(type, "Invalid type pointer");

    const auto iter = mTypesByHash.Find(hash);
    NFE_ASSERT(iter == mTypesByHash.End(), "Type with given hash already exists (%s)", iter->second->GetName().Str());

    mTypesByHash.Insert(hash, type);
    mTypesByName.Insert(StringView(type->GetName()), type);

    type->PrintInfo();

    return type;
}

void TypeRegistry::Cleanup()
{
    NFE_LOG_INFO("RTTI: Cleaning registered types list");

    mTypesByName.Clear();
    mTypesByHash.Clear();
}

} // namespace RTTI
} // namespace NFE
