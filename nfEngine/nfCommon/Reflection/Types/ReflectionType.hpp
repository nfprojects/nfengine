/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "../../nfCommon.hpp"

#include <type_traits>
#include <typeinfo>

// TODO replace with DynArray and String
#include <vector>
#include <string>


namespace NFE {
namespace RTTI {

enum class TypeKind : uint8
{
    Undefined,
    Fundamental,        // fundamental type (int, float, bool, etc.)
    NativeArray,        // T[N] types
    DynArray,           // DynArray<T> types
    UniquePtr,          // UniquePtr<T> types
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
    size_t size;
    size_t alignment;
    TypeKind kind;

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
    NFE_MAKE_NONCOPYABLE(Type);

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
     * Should be called by TypeCreator when all the fields are set up.
     * @note This can be called only once.
     */
    void FinishInitialization(TypeInfo&& info);

    /**
     * Create (allocate and construct) an object of this type.
     * @note    User is responsible for casting to valid type so the appropriate destructor will be used.
     */
    void* CreateObject() const;

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
    // type name (including namespace)
    std::string mName;

    uint32 mSize;
    uint32 mAlignment;

    TypeKind mKind;

    bool mInitialized;
};

using TypePtr = std::unique_ptr<Type>;


} // namespace RTTI
} // namespace NFE
