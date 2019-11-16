/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "../../Containers/UniquePtr.hpp"
#include "../../Containers/String.hpp"

#include <type_traits>
#include <typeinfo>
#include <functional>


namespace NFE {
namespace RTTI {

/**
 * Type of type.
 * Named "type kind" to make it less confiusing.
 */
enum class TypeKind : uint8
{
    Undefined,          // invalid
    Fundamental,        // fundamental type (int, float, bool, etc.)
    Enumeration,        // enum / enum class
    NativeArray,        // T[N] types
    String,
    DynArray,           // DynArray<T> types
    UniquePtr,          // UniquePtr<T> types
    SharedPtr,          // SharedPtr<T> types
    SimpleClass,        // class without virtual methods
    PolymorphicClass,   // class containing at least one virtual method
    AbstractClass,      // class containing at least one pure-virtual method

    // TODO bitfields
};

using ConstructorFunc = std::function<void*()>;
using ArrayConstructorFunc = std::function<void*(uint32)>;

/**
 * Instead of filling Type class directly, this class is used as a helper.
 */
struct TypeInfo
{
public:
    const char* name;
    size_t size;
    size_t alignment;
    TypeKind kind;
    ConstructorFunc constructor;
    ArrayConstructorFunc arrayConstructor;

    TypeInfo()
        : name(nullptr)
        , size(0)
        , alignment(0)
        , kind(TypeKind::Undefined)
    { }
};

/**
 * Generic type information.
 */
class NFCOMMON_API Type
{
    NFE_MAKE_NONCOPYABLE(Type)
    NFE_MAKE_NONMOVEABLE(Type)

public:
    template <typename T>
    friend class TypeCreator;

    explicit Type(const TypeInfo& info);
    virtual ~Type();

    /**
     * Get type name.
     * @note This includes namespaces also.
     */
    NFE_FORCE_INLINE const Common::String& GetName() const { return mName; }

    /**
     * Get type size (in bytes).
     */
    NFE_FORCE_INLINE size_t GetSize() const { return static_cast<size_t>(mSize); }

    /**
     * Get type alignment (in bytes).
     */
    NFE_FORCE_INLINE size_t GetAlignment() const { return static_cast<size_t>(mAlignment); }

    /**
     * Get type kind.
     */
    NFE_FORCE_INLINE TypeKind GetKind() const { return mKind; }

    /**
     * Can be constructed (without arguments)?
     */
    NFE_FORCE_INLINE bool IsConstructible() const { return mConstructor != nullptr; }

    /**
     * Check if this type is compatible with another type.
     */
    virtual bool IsA(const Type* baseType) const;

    /**
     * Convert type kind to string.
     */
    static const char* TypeKindToString(const TypeKind kind);

    /**
     * Print type info into log.
     */
    virtual void PrintInfo() const;

    template<typename T>
    NFE_FORCE_INLINE T* CreateObject() const
    {
        NFE_ASSERT(mConstructor, "Cannot create an object of type '%s'", GetName().Str());
        return static_cast<T*>(CreateRawObject());
    }

    template<typename T>
    NFE_FORCE_INLINE const T* GetDefaultObject() const
    {
        return static_cast<const T*>(mDefaultObject);
    }

    // TODO binary serialization

    /**
     * Write an object of this type to a config value.
     *
     * @param   object              Pointer to a source object of "this" type.
     * @param   config,outValue     Target config value to write.
     * @return  True on success.
     */
    virtual bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const = 0;

    /**
     * Read an object of this type from a config value.
     *
     * @param   outObject       Pointer to a target object of "this" type.
     * @param   config,value    Source config value.
     * @return  True on success.
     */
    virtual bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const = 0;

    /**
     * Deep compare two objects. Returns true if objecs are the same.
     */
    virtual bool Compare(const void* objectA, const void* objectB) const = 0;

protected:

    // allocate and construct object of this type
    void* CreateRawObject() const;

    // type name (including namespace)
    Common::String mName;

    uint32 mSize;
    uint32 mAlignment;

    ConstructorFunc mConstructor;
    ArrayConstructorFunc mArrayConstructor;

    TypeKind mKind;

    const void* mDefaultObject;
};

using TypePtr = Common::UniquePtr<Type>;


} // namespace RTTI
} // namespace NFE
