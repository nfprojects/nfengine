/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system macros for enums.
 */

#pragma once

#include "Types/ReflectionEnumType.hpp"
#include "ReflectionTypeRegistry.hpp"

#include <type_traits>


/**
 * Begin definition of an enum type.
 * @note    Must be ended with NFE_END_DEFINE_ENUM.
 * @note    Must be used outside any namespace.
 */
#define NFE_BEGIN_DEFINE_ENUM(T)                                                                        \
    static_assert(std::is_enum_v<T>, "Given type '" #T "' is not an enum");                             \
    namespace NFE { namespace RTTI {                                                                    \
        namespace { const Type* NFE_UNIQUE_NAME(gForceResolvedType) = GetType<T>(); }                   \
        void TypeCreator<T>::FinishInitialization(EnumTypeInfo& typeInfo)                               \
        {                                                                                               \
            typeInfo.name = #T;                                                                         \
            using EnumType = T; /* EnumType can be used in other NFE_ENUM macros */


/**
 * End definition of enum type.
 */
#define NFE_END_DEFINE_ENUM()           \
        }                               \
    } } /* namespace NFE::RTTI */


/**
 * Register an enum option.
 */
#define NFE_ENUM_OPTION(OptionName) \
    typeInfo.options.PushBack(NFE::RTTI::EnumOption(static_cast<NFE::uint64>(EnumType::OptionName), #OptionName));
