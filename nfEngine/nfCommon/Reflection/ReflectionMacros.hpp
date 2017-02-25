/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system macros.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "ReflectionTypeRegistry.hpp"


/**
 * Get NFE::RTTI::Type object by C++ type.
 */
#define NFE_GET_TYPE(T) (NFE::RTTI::TypeCreator<std::remove_cv<T>::type>::GetType())


/**
 * Declare a type. This must be placed OUTSIDE namespace.
 */
#define NFE_DECLARE_TYPE(T)                                                             \
    namespace NFE {                                                                     \
        namespace RTTI {                                                                \
            template <>                                                                 \
            class TypeCreator<T>                                                        \
            {                                                                           \
            public:                                                                     \
                TypeCreator();                                                          \
                NFE_INLINE static const Type* GetType()                                 \
                {                                                                       \
                    return GetTypeIntenal();                                            \
                }                                                                       \
            private:                                                                    \
                NFE_INLINE static Type* GetTypeIntenal()                                \
                {                                                                       \
                    static Type* type = TypeRegistry::GetInstance().GetType(#T);        \
                    return type;                                                        \
                }                                                                       \
            };                                                                          \
        } /* namespace RTTI */                                                          \
    } /* namespace NFE */


/**
 * Define fundamental type, e.g. 'int'.
 */
#define NFE_DEFINE_FUNDAMENTAL_TYPE_BASE(T, uniqueName)                                                 \
    namespace NFE {                                                                                     \
        namespace RTTI {                                                                                \
        static_assert(std::is_fundamental<T>::value, "NFE::RTTI::Type '" #T "' is not fundamental");    \
                                                                                                        \
            TypeCreator<T> gTypeCreator_##uniqueName;                                                   \
            TypeCreator<T>::TypeCreator()                                                               \
            {                                                                                           \
                TypeInfo typeInfo;                                                                      \
                typeInfo.name = #T;                                                                     \
                typeInfo.size = sizeof(T);                                                              \
                typeInfo.alignment = alignof(T);                                                        \
                GetTypeIntenal()->FinishInitialization(typeInfo);                                       \
            }                                                                                           \
        } /* namespace RTTI */                                                                          \
    } /* namespace NFE */

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



//////////////////////////////////////////////////////////////////////////


/**
 * Declare occurrence of a non-polymorphic class. Must be placed inside class declaration.
 */
#define NFE_DECLARE_CLASS(T)                                                                        \
    friend class NFE::RTTI::TypeCreator<T>;                                                         \
    public:                                                                                         \
        static const NFE::RTTI::Type* GetType();                                                    \
        static const T* GetNullPtr();                                                               \
    private:                                                                                        \
        static bool _InitType(NFE::RTTI::TypeInfo& typeInfo);

/**
 * Declare occurrence of a polymorphic class. Must be placed inside class declaration.
 */
#define NFE_DECLARE_POLYMORPHIC_CLASS(T)                                                            \
    NFE_DECLARE_CLASS(T)                                                                            \
    public:                                                                                         \
        virtual const NFE::RTTI::Type* GetDynamicType() const;

/**
 * Begin definition of a class type.
 * Must be ended with NFE_END_DEFINE_CLASS
 */
#define NFE_BEGIN_DEFINE_CLASS(T)                                                                       \
    namespace NFE {                                                                                     \
        namespace RTTI {                                                                                \
            static_assert(std::is_class<T>::value, "Given type '" #T "' is not a class");               \
                                                                                                        \
            TypeCreator<T> gTypeCreator_##T;                                                            \
            TypeCreator<T>::TypeCreator()                                                               \
            {                                                                                           \
                TypeInfo typeInfo;                                                                      \
                typeInfo.name = #T;                                                                     \
                typeInfo.size = sizeof(T);                                                              \
                typeInfo.alignment = alignof(T);                                                        \
                T::_InitType(typeInfo);                                                                 \
                GetTypeIntenal()->FinishInitialization(typeInfo);                                       \
            }                                                                                           \
        } /* namespace RTTI */                                                                          \
    } /* namespace NFE */                                                                               \
                                                                                                        \
    const NFE::RTTI::Type* T::GetDynamicType() const { return GetType(); }                              \
    const NFE::RTTI::Type* T::GetType() { return NFE::RTTI::TypeCreator<T>::GetType(); }                \
    const T* T::GetNullPtr() { return nullptr; }                                                        \
    bool T::_InitType(NFE::RTTI::TypeInfo& typeInfo)                                                    \
    {


/**
 * End definition of class type.
 */
#define NFE_END_DEFINE_CLASS() return true; } // end of T::_InitType


/**
 * Register class parent.
 */
#define NFE_CLASS_PARENT(ParentType)   \
    static_assert(std::is_class<ParentType>::value, "Given type '" #ParentType "' is not a class");     \
    if (!NFE::RTTI::Type::DefineInheritance(NFE::RTTI::TypeCreator<ParentType>::GetType(), type))       \
        return false;


/**
 * Register class member.
 */
#define NFE_CLASS_MEMBER(memberName)                                                                    \
{                                                                                                       \
    using MemberType = decltype(GetNullPtr()->memberName);                                              \
    static_assert(!std::is_pointer<MemberType>::value, "Member '" #memberName "' is a pointer");        \
    static_assert(!std::is_reference<MemberType>::value, "Member '" #memberName "' is a reference");    \
    typeInfo.members.push_back(NFE::RTTI::Member(#memberName,                                           \
                               reinterpret_cast<size_t>(&(GetNullPtr()->memberName)),                   \
                               NFE_GET_TYPE(MemberType)));                                              \
}
