/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's types registry.
 */

#pragma once

#include "../nfCommon.hpp"

#include <unordered_map>
#include <memory>


namespace NFE {
namespace RTTI {

/**
 * Container for all the registered types.
 */
class NFCOMMON_API TypeRegistry
{
    NFE_MAKE_NONCOPYABLE(TypeRegistry);
    NFE_MAKE_NONMOVEABLE(TypeRegistry);

public:
    /**
     * Get the registry singleton instance.
     */
    static TypeRegistry& GetInstance();

    /**
     * Resolve type by name.
     */
    Type* GetType(const std::string& name);

private:
    TypeRegistry() = default;

    std::unordered_map<std::string, TypePtr> mTypesMap;
};

} // namespace RTTI
} // namespace NFE
