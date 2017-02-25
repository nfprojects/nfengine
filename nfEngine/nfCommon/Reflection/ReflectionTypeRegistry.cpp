/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's type registry.
 */


#include "../PCH.hpp"
#include "ReflectionTypeRegistry.h"
#include "ReflectionType.h"

namespace NFE {
namespace RTTI {

TypeRegistry& TypeRegistry::GetInstance()
{
    static TypeRegistry registry;
    return registry;
}

const Type* TypeRegistry::GetType(const std::string& name) const
{
    const auto iter = mTypesMap.find(name);

    if (iter == mTypesMap.end())
    {
        // type not found
        return nullptr;
    }

    return iter->second;
}

bool TypeRegistry::RegisterType(const Type* type)
{
    const auto iter = mTypesMap.find(type->GetName());

    if (iter != mTypesMap.end())
    {
        // type already registered
        return false;
    }

    mTypesMap[type->GetName()] = type;
    return true;
}

} // namespace RTTI
} // namespace NFE
