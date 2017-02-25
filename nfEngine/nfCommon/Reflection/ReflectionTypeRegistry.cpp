/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's type registry.
 */


#include "PCH.hpp"
#include "ReflectionTypeRegistry.hpp"
#include "ReflectionType.hpp"
#include "../Logger/Logger.hpp"


namespace NFE {
namespace RTTI {

TypeRegistry& TypeRegistry::GetInstance()
{
    static TypeRegistry registry;
    return registry;
}

Type* TypeRegistry::GetType(const std::string& name)
{
    const auto iter = mTypesMap.find(name);

    if (iter == mTypesMap.end())
    {
        // type not found - create a new one
        Type* type = new Type;
        mTypesMap[name] = type;
        return type;
    }

    return iter->second;
}

} // namespace RTTI
} // namespace NFE
