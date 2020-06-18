/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system macros for classes.
 */

#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace RTTI {

struct ClassTypeInfo;

} // namespace RTTI
} // namespace NFE

// Declare occurrence of a non-polymorphic class. Must be placed inside class declaration.
#define NFE_DECLARE_CLASS(T)                                                                \
    friend class NFE::RTTI::TypeCreator<T>;                                                 \
    private:                                                                                \
        static bool _InitType(NFE::RTTI::ClassTypeInfo& typeInfo);


// Declare occurrence of a polymorphic class. Must be placed inside class declaration.
#define NFE_DECLARE_POLYMORPHIC_CLASS(T)                                                    \
    friend class NFE::RTTI::TypeCreator<T>;                                                 \
    public:                                                                                 \
        virtual const NFE::RTTI::Type* GetDynamicType() const;                              \
    private:                                                                                \
        static bool _InitType(NFE::RTTI::ClassTypeInfo& typeInfo);

#include "Object.hpp"
