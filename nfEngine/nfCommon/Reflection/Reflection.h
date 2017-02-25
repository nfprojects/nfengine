/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system macros.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ReflectionType.h"


// remove 'const' and 'volatile' decorators from given C++ type
#define NFE_REMOVE_CONST_AND_VOLATILE(T) std::remove_cv<T>::type


//////////////////////////////////////////////////////////////////////////


/**
 * Get NFE::RTTI::Type object by C++ type.
 * Useful for obtaining type information for primitive types, e.g. NFE_GET_TYPE(int).
 */
#define NFE_GET_TYPE(T) (NFE::RTTI::TypeCreator<NFE_REMOVE_CONST_AND_VOLATILE(T)>::Get())


//////////////////////////////////////////////////////////////////////////


/**
 * Define fundamental type, e.g. 'int'.
 */
#define NFE_DEFINE_FUNDAMENTAL_TYPE_BASE(T, uniqueName)                                             \
    static_assert(std::is_fundamental<T>::value, "NFE::RTTI::Type '" #T "' is not fundamental");    \
    template<>                                                                                      \
    bool NFE::RTTI::TypeCreator<T>::Initialize(NFE::RTTI::Type* type)                               \
    {                                                                                               \
        type->mName = #T;                                                                           \
        type->mSize = sizeof(T);                                                                    \
        type->mAlignment = alignof(T);                                                              \
        return true;                                                                                \
    }                                                                                               \
    NFE::RTTI::TypeCreator<T> gTypeCreator_##uniqueName

/**
 * Define fundamental type, e.g. 'int'.
 */
#define NFE_DEFINE_FUNDAMENTAL_TYPE(T) \
    NFE_DEFINE_FUNDAMENTAL_TYPE_BASE(T, T)

/**
 * Define fundamental type inside namespace, e.g. 'NFE::int'.
 */
#define NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE(N, T) \
    NFE_DEFINE_FUNDAMENTAL_TYPE_BASE(N::T, N##_##T)

 /**
 * Define fundamental type inside double namespace, e.g. 'NFE::ABC::int'.
 */
#define NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE2(N1, N2, T) \
    NFE_DEFINE_FUNDAMENTAL_TYPE_BASE(N1::N2::T, N1##_##N2##_##T)

// TODO are more for triple namespace if needed or write some better, generic macro


 //////////////////////////////////////////////////////////////////////////


/**
 * Declare occurrence of a class. Must be placed inside class declaration.
 */
#define NFE_DECLARE_CLASS(T)                                                                        \
    friend class NFE::RTTI::TypeCreator<T>;                                                         \
    public:                                                                                         \
        virtual const NFE::RTTI::Type* GetDynamicType() const;                                      \
        static const NFE::RTTI::Type* GetType();                                                    \
        static const T* GetNullPtr();                                                               \
    private:                                                                                        \
        static bool _InitType(NFE::RTTI::Type* type);



/**
 * Begin definition of a class type.
 * Must be ended with NFE_END_CLASS
 */
#define NFE_BEGIN_CLASS(T)                                                                          \
    static_assert(std::is_class<T>::value, "Given type '" #T "' is not a class");                   \
                                                                                                    \
    NFE::RTTI::TypeCreator<T> gTypeCreator_##T;                                                     \
    template<>                                                                                      \
    bool NFE::RTTI::TypeCreator<T>::Initialize(NFE::RTTI::Type* type)                               \
    {                                                                                               \
        type->mName = #T;                                                                           \
        type->mSize = sizeof(T);                                                                    \
        type->mAlignment = alignof(T);                                                              \
        return T::_InitType(type);                                                                  \
    }                                                                                               \
    const NFE::RTTI::Type* T::GetDynamicType() const { return NFE::RTTI::TypeCreator<T>::Get(); }   \
    const NFE::RTTI::Type* T::GetType() { return NFE::RTTI::TypeCreator<T>::Get(); }                \
    const T* T::GetNullPtr() { return nullptr; }                                                    \
    bool T::_InitType(NFE::RTTI::Type* type)                                                        \
    {



/**
 * End definition of class type.
 */
#define NFE_END_CLASS() return true; } // end of T::_InitType



/**
 * Register class parent.
 */
#define NFE_CLASS_PARENT(ParentType)   \
    static_assert(std::is_class<ParentType>::value, "Given type '" #ParentType "' is not a class");     \
    if (!NFE::RTTI::Type::DefineInheritance(NFE_GET_TYPE(ParentType), type))                            \
        return false;



/**
 * Register class member.
 */
#define NFE_CLASS_MEMBER(memberName)                                                                    \
{                                                                                                       \
    using MemberType = decltype(GetNullPtr()->memberName);                                              \
    static_assert(!std::is_pointer<MemberType>::value, "Member '" #memberName "' is a pointer");        \
    static_assert(!std::is_reference<MemberType>::value, "Member '" #memberName "' is a reference");    \
    type->mMembers.push_back(NFE::RTTI::Member(#memberName,                                             \
                                    reinterpret_cast<size_t>(&(GetNullPtr()->memberName)),              \
                                    NFE_GET_TYPE(MemberType)));                                         \
}
