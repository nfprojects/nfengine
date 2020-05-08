/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection utilities.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ReflectionTypeResolver.hpp"
#include "Types/ReflectionType.hpp"
#include "../Containers/SharedPtr.hpp"


namespace NFE {
namespace RTTI {

/**
 * Get default object for a given C++ type.
 * Note: returns nullptr if the type is not default-constructible.
 */
template<typename T>
const T* GetDefaultObject()
{
    static_assert(std::is_default_constructible_v<T>, "Given type is not default-constructible");

    if (const Type* type = GetType<T>())
    {
        return type->GetDefaultObject<T>();
    }

    return nullptr;
}

/**
 * Compare two objects.
 */
template<typename T>
bool Compare(const T& objectA, const T& objectB)
{
    const Type* type = GetType<T>();
    NFE_ASSERT(type, "Unknown type");

    return type->Compare(&objectA, &objectB);
}

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
            return static_cast<TargetType*>(source);
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
            return static_cast<const TargetType*>(source);
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
Common::SharedPtr<TargetType> Cast(const Common::SharedPtr<SourceType>& source)
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
            return Common::StaticCast<TargetType>(source);
        }
    }

    // incompatible types
    return nullptr;
}

} // namespace RTTI
} // namespace NFE
