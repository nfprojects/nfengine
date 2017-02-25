/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's types registry.
 */

#pragma once

#include "../nfCommon.hpp"
#include "Types/ReflectionType.hpp"
#include "Types/ReflectionClassType.hpp"

#include <unordered_map>
#include <memory>


namespace NFE {
namespace RTTI {

/**
 * Container for all the registered types.
 */
class NFCOMMON_API TypeRegistry
{
    NFE_MAKE_NONCOPYABLE(TypeRegistry)
    NFE_MAKE_NONMOVEABLE(TypeRegistry)

public:
    /**
     * Get the registry singleton instance.
     */
    static TypeRegistry& GetInstance();

    /**
     * Find existing type by typeid hash.
     */
    const Type* GetExistingType(size_t hash) const;

    /**
     * Find existing type by name.
     */
    const Type* GetExistingTypeByName(const std::string& name) const;

    /**
     * Register non-existing type.
     */
    const Type* RegisterType(size_t hash, TypePtr&& type);

    /**
     * Unregister all the types.
     */
    void Cleaup();

private:
    TypeRegistry() = default;

    std::unordered_map<std::string, const Type*> mTypesByName;
    std::unordered_map<size_t, TypePtr> mTypesByHash;
};


template <typename T>
typename std::enable_if<!std::is_constructible<T>::value, ConstructorFunc>::type GetObjectConstructor()
{
    // abstract object can't be constructed
    return ConstructorFunc();
}

/**
 * Get object constructor for non-abstract type.
 */
template <typename T>
typename std::enable_if<std::is_constructible<T>::value, ConstructorFunc>::type GetObjectConstructor()
{
    return []() { return new T; };
}

template <typename T>
typename std::enable_if<!std::is_constructible<T>::value, ArrayConstructorFunc>::type GetArrayConstructor()
{
    // array of abstract objects can't be constructed
    return ArrayConstructorFunc();
}

/**
 * Get array constructor for non-abstract type.
 */
template <typename T>
typename std::enable_if<std::is_constructible<T>::value, ArrayConstructorFunc>::type GetArrayConstructor()
{
    return [](uint32 arraySize) { return new T[arraySize]; };
}


/**
 * Template class used to generate Type class instances.
 * This template will be specialized in NFE_DECLARE_TYPE macro.
 */
template <typename T>
class TypeCreator
{
public:
    using TypeClass = ClassType;
    using TypeInfoClass = ClassTypeInfo;

    static TypePtr CreateType()
    {
        ClassTypeInfo typeInfo;
        typeInfo.kind = TypeKind::SimpleClass; // can be overridden in FinishInitialization
        typeInfo.size = sizeof(T);
        typeInfo.alignment = std::alignment_of<T>::value;
        typeInfo.constructor = GetObjectConstructor<T>();
        typeInfo.arrayConstructor = GetArrayConstructor<T>();

        TypeCreator creator;
        creator.FinishInitialization(typeInfo);
        return TypePtr(new ClassType(typeInfo));
    }

    void FinishInitialization(TypeInfoClass& typeInfo);
};


/**
 * Find existing type object or create new one.
 */
template<typename T>
const Type* ResolveType()
{
    static_assert(!std::is_const<T>::value && !std::is_volatile<T>::value, "Resolved type must be without any decorators");

    const size_t hash = typeid(T).hash_code();
    const Type* existingType = TypeRegistry::GetInstance().GetExistingType(hash);
    if (existingType)
    {
        return existingType;
    }

    TypePtr newType = TypeCreator<T>::CreateType();
    existingType = TypeRegistry::GetInstance().RegisterType(hash, std::move(newType));

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
    * Create (allocate and construct) an object of this type.
    */
template <typename T>
T* CreateObject()
{
    const Type* type = GetType<T>();
    return reinterpret_cast<T*>(type->CreateObject());
}

} // namespace RTTI
} // namespace NFE
