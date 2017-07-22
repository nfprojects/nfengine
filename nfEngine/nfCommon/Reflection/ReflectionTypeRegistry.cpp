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


TypeRegistry& TypeRegistry::GetInstance()
{
    static TypeRegistry registry;
    return registry;
}

const Type* TypeRegistry::GetExistingType(size_t hash) const
{
    const auto iter = mTypesByHash.find(hash);

    if (iter == mTypesByHash.end())
    {
        return nullptr;
    }

    return iter->second.Get();
}

const Type* TypeRegistry::GetExistingType(const std::string& name) const
{
    const auto iter = mTypesByName.find(name);

    if (iter == mTypesByName.end())
    {
        return nullptr;
    }

    return iter->second;
}

const Type* TypeRegistry::RegisterType(size_t hash, TypePtr&& type)
{
    NFE_ASSERT(type, "Invalid type pointer");

    const auto iter = mTypesByHash.find(hash);
    NFE_ASSERT(iter == mTypesByHash.end(), "Type with given hash already exists (%s)", iter->second->GetName());

    const Type* typePtr = type.Get();
    mTypesByHash[hash] = std::move(type);
    mTypesByName[typePtr->GetName()] = typePtr;

    typePtr->PrintInfo();

    return typePtr;
}

void TypeRegistry::Cleanup()
{
    mTypesByName.clear();
    mTypesByHash.clear();
}

} // namespace RTTI
} // namespace NFE
