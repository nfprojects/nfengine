/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "../../nfCommon.hpp"

#include <type_traits>
#include <typeinfo>
#include <string>
#include <functional>


namespace NFE {
namespace RTTI {

/**
 * Type of type.
 */
enum class TypeKind : uint8
{
    Undefined,          // invalid
    Fundamental,        // fundamental type (int, float, bool, etc.)
    NativeArray,        // T[N] types
    DynArray,           // DynArray<T> types
    UniquePtr,          // UniquePtr<T> types
    SharedPtr,          // SharedPtr<T> types
    SimpleClass,        // class without virtual methods
    PolymorphicClass,   // class containing at least one virtual method
    AbstractClass,      // class containing at least one pure-virtual method

    // TODO enums, bitfields
};

using ConstructorFunc = std::function<void*()>;
using ArrayConstructorFunc = std::function<void*(uint32)>;

/**
 * Instead of filling Type class directly, this class is used as a helper.
 */
struct TypeInfo
{
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

public:
    template <typename T>
    friend class TypeCreator;

    Type(const TypeInfo& info);
    virtual ~Type() { }

    /**
     * Get type name.
     * @note This includes namespaces also.
     */
    const char* GetName() const { return mName.c_str(); }

    /**
     * Get type size (in bytes).
     */
    size_t GetSize() const { return static_cast<size_t>(mSize); }

    /**
     * Get type alignment (in bytes).
     */
    size_t GetAlignment() const { return static_cast<size_t>(mAlignment); }

    /**
     * Get type kind.
     */
    TypeKind GetKind() const { return mKind; }

    /**
     * Can be constructed (without arguments)?
     */
    bool IsConstructible() const { return mConstructor != nullptr; }

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
    T* CreateObject() const
    {
        return reinterpret_cast<T*>(CreateRawObject());
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

protected:
    // allocate and construct object of this type
    void* CreateRawObject() const;

    // type name (including namespace)
    std::string mName;

    uint32 mSize;
    uint32 mAlignment;

    ConstructorFunc mConstructor;
    ArrayConstructorFunc mArrayConstructor;

    TypeKind mKind;

    bool mInitialized;
};

using TypePtr = std::unique_ptr<Type>;


} // namespace RTTI
} // namespace NFE
