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
        Type* type = new Type;
        mTypesMap[name] = std::unique_ptr<Type>(type);
        return type;
    }

    return iter->second.get();
}

} // namespace RTTI
} // namespace NFE
