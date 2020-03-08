/**
 * @file
 * @brief  Declaration of reflection system's type resolver.
 */

#pragma once

#include "../nfCommon.hpp"

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


using ConstructorFunc = std::function<void* ()>;
using ArrayConstructorFunc = std::function<void* (uint32)>;


template <typename T>
typename std::enable_if<!std::is_constructible_v<T>, ConstructorFunc>::type GetObjectConstructor()
{
    // abstract object can't be constructed
    return ConstructorFunc();
}

/**
 * Get object constructor for non-abstract type.
 */
template <typename T>
typename std::enable_if<std::is_constructible_v<T>, ConstructorFunc>::type GetObjectConstructor()
{
    return []() { return new T; };
}

template <typename T>
typename std::enable_if<!std::is_constructible_v<T>, ArrayConstructorFunc>::type GetArrayConstructor()
{
    // array of abstract objects can't be constructed
    return ArrayConstructorFunc();
}

/**
 * Get array constructor for non-abstract type.
 */
template <typename T>
typename std::enable_if<std::is_constructible_v<T>, ArrayConstructorFunc>::type GetArrayConstructor()
{
    return [](uint32 arraySize) { return new T[arraySize]; };
}



/**
 * Find existing type object or create new one.
 */
template<typename T>
const Type* ResolveType()
{
    static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Resolved type must be without any decorators");

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
    // cache the type pointer so it's resolved only once
    static const Type* type = ResolveType<T>();

    // TODO this cast could be done in TypeCreator already
    return static_cast<const typename TypeCreator<T>::TypeClass*>(type);
}


/**
 * Get default object for a given C++ type.
 * Note: returns nullptr if the type is not default-constructible.
 */
template<typename T>
const T* GetDefaultObject()
{
    static_assert(std::is_default_constructible_v<T>, "Given type is not default-constructible");

    if (const auto* type = GetType<T>())
    {
        return type->GetDefaultObject<T>();
    }

    return nullptr;
}


} // namespace RTTI
} // namespace NFE
