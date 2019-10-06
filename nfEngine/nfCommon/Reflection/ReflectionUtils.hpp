/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection utilities.
 */

#pragma once

#include "../nfCommon.hpp"
#include "Types/ReflectionType.hpp"


namespace NFE {
namespace RTTI {

/**
 * Dynamic cast helper function (non-const version).
 * @notes   This function work only with class types.
 */
template<typename TargetType, typename SourceType>
TargetType* Cast(SourceType* source)
{
    static_assert(std::is_class_v<SourceType>, "Source type is not class type");
    static_assert(std::is_class_v<TargetType>, "Target type is not class type");

    if (source)
    {
        const Type* sourceType = source->GetDynamicType();
        const Type* targetType = GetType<TargetType>();

        // TODO multiple inheritance support
        if (sourceType->IsA(targetType))
        {
            return reinterpret_cast<TargetType*>(source);
        }
    }

    // incompatible types
    return nullptr;
}

/**
 * Dynamic cast helper function (const version).
 * @notes   This function work only with class types.
 */
template<typename TargetType, typename SourceType>
const TargetType* Cast(const SourceType* source)
{
    static_assert(std::is_class_v<SourceType>, "Source type is not class type");
    static_assert(std::is_class_v<TargetType>, "Target type is not class type");

    if (source)
    {
        const Type* sourceType = source->GetDynamicType();
        const Type* targetType = GetType<TargetType>();

        // TODO multiple inheritance support
        if (sourceType->IsA(targetType))
        {
            return reinterpret_cast<const TargetType*>(source);
        }
    }

    // incompatible types
    return nullptr;
}

} // namespace RTTI
} // namespace NFE
