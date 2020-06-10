/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system macros for classes.
 */

#pragma once

#include "Types/ReflectionClassType.hpp"

// Declare occurrence of a non-polymorphic class. Must be placed inside class declaration.
#define NFE_DECLARE_CLASS(T)                                                                \
    friend class NFE::RTTI::TypeCreator<T>;                                                 \
    private:                                                                                \
        static const char* _GetName();                                                      \
        static bool _InitType(NFE::RTTI::ClassTypeInfo& typeInfo);


// Declare occurrence of a polymorphic class. Must be placed inside class declaration.
#define NFE_DECLARE_POLYMORPHIC_CLASS(T)                                                    \
    friend class NFE::RTTI::TypeCreator<T>;                                                 \
    public:                                                                                 \
        virtual const NFE::RTTI::Type* GetDynamicType() const;                              \
    private:                                                                                \
        static const char* _GetName();                                                      \
        static bool _InitType(NFE::RTTI::ClassTypeInfo& typeInfo);

#include "Object.hpp"