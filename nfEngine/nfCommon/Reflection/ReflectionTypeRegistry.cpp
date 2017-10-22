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

    return iter->second.Get();
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

const Type* TypeRegistry::RegisterType(size_t hash, TypePtr&& type)
{
    NFE_ASSERT(type, "Invalid type pointer");

    const auto iter = mTypesByHash.Find(hash);
    NFE_ASSERT(iter == mTypesByHash.End(), "Type with given hash already exists (%s)", iter->second->GetName());

    const Type* typePtr = type.Get();
    mTypesByHash.Insert(hash, std::move(type));
    mTypesByName.Insert(StringView(typePtr->GetName()), typePtr);

    typePtr->PrintInfo();

    return typePtr;
}

void TypeRegistry::Cleanup()
{
    NFE_LOG_INFO("RTTI: Cleaning registered types list");

    mTypesByName.Clear();
    mTypesByHash.Clear();
}

} // namespace RTTI
} // namespace NFE
