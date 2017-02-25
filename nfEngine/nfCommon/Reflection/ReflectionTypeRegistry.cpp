/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's type registry.
 */


#include "../PCH.hpp"
#include "ReflectionTypeRegistry.h"
#include "ReflectionType.h"
#include "../Logger.hpp"


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
        // type not found - create a new one
        const Type* type = new Type;
        mTypesMap[name] = type;
        return type;
    }

    return iter->second;
}

} // namespace RTTI
} // namespace NFE
