/**
 * @file
 * @brief  Declaration of reflection system's type resolver.
 */

#pragma once

#include "Types/ReflectionType.hpp"
#include "../System/Assertion.hpp"
#include "../Utils/LanguageUtils.hpp"

#include <type_traits>
#include <functional>

namespace NFE {
namespace RTTI {


class NFCOMMON_API ITypeRegistry
{
    NFE_MAKE_NONCOPYABLE(ITypeRegistry)
    NFE_MAKE_NONMOVEABLE(ITypeRegistry)

public:
    ITypeRegistry() = default;

    virtual ~ITypeRegistry() { }

    static ITypeRegistry& GetInstance();

    // Find existing type by typeid hash.
    virtual const Type* GetExistingType(size_t hash) const = 0;

    // Find existing type by name.
    virtual const Type* GetExistingType(const char* name) const = 0;

    // Register non-existing type.
    virtual const Type* RegisterType(size_t hash, Type* type) = 0;
};


using ConstructorFunc = std::function<void*(void)>;
using DestructorFunc = std::function<void(void*)>;

template <typename T>
typename std::enable_if<!std::is_constructible_v<T>, ConstructorFunc>::type GetObjectConstructor()
{
    // abstract object can't be constructed
    return ConstructorFunc();
}

template <typename T>
typename std::enable_if<!std::is_constructible_v<T>, DestructorFunc>::type GetObjectDestructor()
{
    // abstract object can't be constructed
    return DestructorFunc();
}

// Get object constructor for non-abstract type.
template <typename T>
typename std::enable_if<std::is_constructible_v<T>, ConstructorFunc>::type GetObjectConstructor()
{
    return [] ()
    {
        return new T;
    };
}

// Get object destructor for non-abstract type.
template <typename T>
typename std::enable_if<std::is_constructible_v<T>, DestructorFunc>::type GetObjectDestructor()
{
    return [] (void* ptr)
    {
        T* typedPtr = BitCast<T*>(ptr);
        delete typedPtr;
    };
}


/**
 * Find existing type object or create new one.
 */
template<typename T>
const Type* ResolveType()
{
    static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Resolved type must have no decorators");

    const size_t hash = typeid(T).hash_code();
    const Type* existingType = ITypeRegistry::GetInstance().GetExistingType(hash);
    if (!existingType)
    {
        Type* newType = TypeCreator<T>::CreateType();
        existingType = ITypeRegistry::GetInstance().RegisterType(hash, std::move(newType));
    }

    return existingType;
}

/**
 * Get NFE::RTTI::Type object from a C++ type.
 */
template<typename T>
const typename TypeCreator<T>::TypeClass* GetType()
{
    static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Type should not have any qualifiers");
    static_assert(IsTypeDefined<T>, "Type is not defined");
    static_assert(IsTypeDefined<TypeCreator<T>>, "Type is not defined");

    // cache the type pointer so it's resolved only once
    static const Type* type = ResolveType<T>();

    // TODO this cast could be done in TypeCreator already
    return static_cast<const typename TypeCreator<T>::TypeClass*>(type);
}


} // namespace RTTI
} // namespace NFE
