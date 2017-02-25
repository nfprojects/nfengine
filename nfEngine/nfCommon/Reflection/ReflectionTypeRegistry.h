/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's types registry.
 */

#pragma once

#include "../nfCommon.hpp"

#include <unordered_map>


namespace NFE {
namespace RTTI {

class Type;

/**
 * Container for all the registered types.
 */
class NFCOMMON_API TypeRegistry
{
public:
    /**
     * Get the registry singleton instance.
     */
    static TypeRegistry& GetInstance();

    /**
     * Resolve type by name.
     */
    const Type* GetType(const std::string& name) const;

    /**
     * Add a type to the registry.
     */
    bool RegisterType(const Type* type);

private:
    NFE_INLINE TypeRegistry() { }

    std::unordered_map<std::string, const Type*> mTypesMap;
};

} // namespace RTTI
} // namespace NFE
