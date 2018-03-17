/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system macros for classes.
 */

#pragma once

#include "Types/ReflectionType.hpp"
#include "ReflectionTypeRegistry.hpp"

#include <type_traits>


namespace NFE {
namespace RTTI {

/**
 * Calculate offset of a member within a class.
 */
template <typename Member, typename Class>
NFE_INLINE constexpr size_t OffsetOf(Member Class::*member)
{
    // Workaround for built-in offsetof() function on GCC that does not work with non-standard-layout types.
    constexpr const size_t fakePointer = 0x10; // use non-zero value so it's not treated as nullptr_t
    return reinterpret_cast<size_t>(reinterpret_cast<const char*>(&(reinterpret_cast<const Class*>(fakePointer)->*member))) - fakePointer;
}

} // RTTI
} // NFE


//////////////////////////////////////////////////////////////////////////


/**
 * Declare occurrence of a non-polymorphic class. Must be placed inside class declaration.
 */
#define NFE_DECLARE_CLASS(T)                                                                \
    friend class NFE::RTTI::TypeCreator<T>;                                                 \
    private:                                                                                \
        static const char* _GetName();                                                      \
        static bool _InitType(NFE::RTTI::ClassTypeInfo& typeInfo);


/**
 * Declare occurrence of a polymorphic class. Must be placed inside class declaration.
 */
#define NFE_DECLARE_POLYMORPHIC_CLASS(T)                                                    \
    friend class NFE::RTTI::TypeCreator<T>;                                                 \
    private:                                                                                \
        static const char* _GetName();                                                      \
        static bool _InitType(NFE::RTTI::ClassTypeInfo& typeInfo);                          \
    public:                                                                                 \
        virtual const NFE::RTTI::Type* GetDynamicType() const;


//////////////////////////////////////////////////////////////////////////


/**
 * Begin definition of a class type.
 * @note    Must be ended with NFE_END_DEFINE_CLASS.
 * @note    Must be used outside any namespace.
 */
#define NFE_BEGIN_DEFINE_CLASS(T)                                                                       \
    static_assert(std::is_class<T>::value, "Given type '" #T "' is not a class");                       \
    static_assert(!std::is_polymorphic<T>::value, "Given type '" #T "' is polymorphic");                \
    namespace NFE { namespace RTTI {                                                                    \
        namespace { const Type* NFE_UNIQUE_NAME(gForceResolvedType) = GetType<T>(); }                   \
        template<> NFE_API_EXPORT void TypeCreator<T>::FinishInitialization(ClassTypeInfo& typeInfo)    \
        {                                                                                               \
            typeInfo.name = #T;                                                                         \
            T::_InitType(typeInfo);                                                                     \
        }                                                                                               \
    } } /* namespace NFE::RTTI */                                                                       \
    const char* T::_GetName() { return #T; }                                                            \
    bool T::_InitType(NFE::RTTI::ClassTypeInfo& typeInfo)                                               \
    {                                                                                                   \
        (void)typeInfo;                                                                                 \
        using ClassType = T; /* ClassType can be used in other NFE_CLASS macros */


/**
 * Begin definition of a polymorphic class type.
 * @note    Must be ended with NFE_END_DEFINE_CLASS.
 * @note    Must be used outside any namespace.
 */
#define NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS(T)                                                                   \
    static_assert(std::is_class<T>::value, "Given type '" #T "' is not a class");                               \
    static_assert(std::is_polymorphic<T>::value, "Given type '" #T "' is not polymorphic");                     \
    static_assert(std::has_virtual_destructor<T>::value, "Use virtual destructor for polymorphic classes!");    \
    namespace NFE { namespace RTTI {                                                                            \
        namespace { const Type* NFE_UNIQUE_NAME(gForceResolvedType) = GetType<T>(); }                           \
        template<> NFE_API_EXPORT void TypeCreator<T>::FinishInitialization(ClassTypeInfo& typeInfo)            \
        {                                                                                                       \
            typeInfo.kind = std::is_abstract<T>::value ? TypeKind::AbstractClass : TypeKind::PolymorphicClass;  \
            typeInfo.name = #T;                                                                                 \
            T::_InitType(typeInfo);                                                                             \
        }                                                                                                       \
    } } /* namespace NFE::RTTI */                                                                               \
    const char* T::_GetName() { return #T; }                                                                    \
    const NFE::RTTI::Type* T::GetDynamicType() const { return NFE::RTTI::GetType<T>(); }                        \
    bool T::_InitType(NFE::RTTI::ClassTypeInfo& typeInfo)                                                       \
    {                                                                                                           \
        (void)typeInfo;                                                                                         \
        using ClassType = T; /* ClassType can be used in other NFE_CLASS macros */


//////////////////////////////////////////////////////////////////////////


/**
 * End definition of class type.
 */
#define NFE_END_DEFINE_CLASS() return true; } // end of T::_InitType


/**
 * Register class parent.
 */
#define NFE_CLASS_PARENT(ParentType)   \
    static_assert(std::is_class<ParentType>::value, "Given type '" #ParentType "' is not a class");                                 \
    static_assert(std::is_base_of<ParentType, ClassType>::value, "Given type '" #ParentType "' is not a parent of defined class");  \
    typeInfo.parent = NFE::RTTI::GetType<ParentType>();


/**
 * Register class member.
 */
#define NFE_CLASS_MEMBER(memberName)                                                            \
{                                                                                               \
    /* extract member compile-time type */                                                      \
    using MemberType = decltype(static_cast<const ClassType*>(nullptr)->memberName);            \
    static_assert(!std::is_pointer<MemberType>::value, "Member must not be a raw pointer");     \
    static_assert(!std::is_reference<MemberType>::value, "Member must not be a reference");     \
    /* register the member */                                                                   \
    typeInfo.members.PushBack(NFE::RTTI::Member(#memberName,                                    \
                                                NFE::RTTI::OffsetOf(&ClassType::memberName),    \
                                                NFE::RTTI::GetType<MemberType>()));             \
}
