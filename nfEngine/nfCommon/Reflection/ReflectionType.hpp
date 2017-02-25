/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ReflectionMember.hpp"
#include "ReflectionTypeRegistry.hpp"
#include "ReflectionTypeSerializer.hpp"

// TODO replace with DynArray and String
#include <vector>
#include <string>


namespace NFE {
namespace RTTI {

enum class TypeKind : uint8
{
    Undefined,
    Fundamental,        // fundamental type (int, float, bool, etc.)
    SimpleClass,        // class without virtual methods
    PolymorphicClass,   // class containing at least one virtual method
    AbstractClass,      // class containing at least one pure-virtual method

    // TODO enums, bitfields
};

/**
 * Instead of filling Type class directly, this class is used as a helper.
 */
struct TypeInfo
{
    const char* name;
    const Type* parent;
    size_t size;
    size_t alignment;
    std::vector<const Type*> childTypes;
    std::vector<Member> members;
    std::unique_ptr<ITypeSerializer> serializer;
    TypeKind kind;
    bool isTriviallyConstructible;

    NFE_INLINE TypeInfo()
        : name(nullptr)
        , parent(nullptr)
        , size(0)
        , alignment(0)
        , kind(TypeKind::Undefined)
        , isTriviallyConstructible(true)
    { }
};

/**
 * Type information.
 * Can be a primitive type as well as a complex class.
 */
class NFCOMMON_API Type
{
    NFE_MAKE_NONCOPYABLE(Type);

public:
    template <typename T>
    friend class TypeCreator;

    using Members = std::vector<Member>;
    using Children = std::vector<const Type*>;

    Type(const char* name);
    Type(Type&&) = default;
    Type& operator=(Type&&) = default;

    NFE_INLINE const char* GetName() const { return mName.c_str(); }
    NFE_INLINE size_t GetSize() const { return static_cast<size_t>(mSize); }
    NFE_INLINE size_t GetAlignment() const { return static_cast<size_t>(mAlignment); }
    NFE_INLINE const Type* GetParent() const { return mParent; }
    NFE_INLINE TypeKind GetKind() const { return mKind; }

    /**
     * Check if this type is derived from the other type.
     */
    static bool IsBaseOf(const Type* baseType, const Type* derivedType);

    /**
     * Should be called by TypeCreator when all the fields are set up.
     * @note This can be called only once.
     */
    void FinishInitialization(TypeInfo&& info);

    /**
     * Enumerate all subtypes of this type (including self).
     */
    void ListSubtypes(std::vector<const Type*>& outTypes) const;

    /**
     * Get number of member (including members that come from parent type).
     */
    size_t GetNumOfMembers() const;

    /**
     * Enumerate all members (including members that come from parent type).
     */
    void ListMembers(std::vector<Member>& outMembers) const;

    /**
     * Write an object of this type to a config value.
     *
     * @param   object              Pointer to a source object of "this" type.
     * @param   config,outValue     Target config value to write.
     * @return  True on success.
     */
    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const;

    /**
     * Read an object of this type from a config value.
     *
     * @param   outObject       Pointer to a target object of "this" type.
     * @param   config,value    Source config value.
     * @return  True on success.
     */
    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const;

    /**
     * Create an object of this type using default (trivial) constructor.
     * @return  Valid object pointer or nullptr if the type is not trivially constructible.
     */
    void* CreateObjectRaw() const;

    template<typename T>
    T* CreateObject() const { return reinterpret_cast<T*>(CreateObjectRaw()); }

private:
    // type name (including namespace)
    std::string mName;

    // for serialization
    std::unique_ptr<ITypeSerializer> mSerializer;

    // list of members
    Members mMembers;

    // parent + list of child types
    const Type* mParent;
    Children mChildTypes;

    uint32 mSize;
    uint32 mAlignment;

    TypeKind mKind;

    bool mInitialized;

    bool SerializeClassType(const void* object, Common::Config& config, Common::ConfigValue& outValue) const;
    bool DeserializeClassType(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const;
};


/**
 * Template class used to generate Type class instances.
 * This template will be specialized in NFE_DECLARE_TYPE macro.
 */
template <typename T>
class TypeCreator
{
public:
    static const Type* GetType()
    {
        // if you get this error, this means you didn't use NFE_DECLARE_TYPE(T)
        // or didn't include proper header containing these macros
        static_assert(false, "Type is not declared");
        return nullptr;
    }

private:
    static const Type* GetTypeIntenal()
    {
        return nullptr;
    }
};

} // namespace RTTI
} // namespace NFE
