/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection utilities.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ReflectionType.hpp"


namespace NFE {
namespace RTTI {

/**
 * Dynamic cast helper function.
 */
template<typename SourceType, typename TargetType>
TargetType* Cast(SourceType* object)
{
    const Type* sourceType = object->GetDynamicType();
    const Type* targetType = GetType<TargetType>();

    // TODO multiple inheritance support
    if (Type::IsBaseOf(sourceType, targetType) || Type::IsBaseOf(targetType, sourceType))
    {
        return static_cast<targetType*>(object);
    }

    // incompatible types
    return nullptr;
}

} // namespace RTTI
} // namespace NFE
