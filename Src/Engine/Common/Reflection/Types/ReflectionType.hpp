/**
 * @file
 * @author Witek902
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "../../Containers/String.hpp"

#include <functional>


namespace NFE {
namespace RTTI {

/**
 * Type of type.
 * Named "type kind" to make it less confusing.
 */
enum class TypeKind : uint8
{
    Undefined = 0,      // invalid
    Fundamental,        // fundamental type (int, float, bool, etc.)
    Enumeration,        // enum / enum class
    NativeArray,        // T[N] types
    String,             // NFE::String type
    DynArray,           // NFE::DynArray<T> types
    UniquePtr,          // NFE::UniquePtr<T> types
    SharedPtr,          // NFE::SharedPtr<T> types
    Class,              // class type

    // TODO bitfields
};

using ConstructorFunc = std::function<void(void*)>;
using DestructorFunc = std::function<void(void*)>;

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
    DestructorFunc destructor;

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

    Type();
    virtual ~Type();

    // finish type initialization
    void Initialize(const TypeInfo& info);

    // Get type name (his includes namespaces also)
    NFE_FORCE_INLINE const Common::String& GetName() const { return mName; }

    // Get type size (in bytes)
    NFE_FORCE_INLINE size_t GetSize() const { return static_cast<size_t>(mSize); }

    // Get type alignment (in bytes)
    NFE_FORCE_INLINE size_t GetAlignment() const { return static_cast<size_t>(mAlignment); }

    // Get type kind.
    NFE_FORCE_INLINE TypeKind GetKind() const { return mKind; }

    // Can be constructed (without arguments)?
    NFE_FORCE_INLINE bool IsConstructible() const { return mConstructor != nullptr; }

    // Check if this type is compatible with another type (given type pointer)
    virtual bool IsA(const Type* baseType) const;

    // Convert type kind to string.
    static const char* TypeKindToString(const TypeKind kind);

    // Print type info into log.
    virtual void PrintInfo() const;

    // allocate and construct object of this type
    [[nodiscard]] void* CreateRawObject() const;

    // construct an object given allocated memory
    void ConstructObject(void* objectPtr) const;

    // destruct an object
    void DestructObject(void* objectPtr) const;

    // allocate memory and construct object
    template<typename T>
    [[nodiscard]] NFE_FORCE_INLINE T* CreateObject() const
    {
        return static_cast<T*>(CreateRawObject());
    }

    // destroy object and release memory
    void DeleteObject(void* objectPtr) const;

    template<typename T>
    NFE_FORCE_INLINE const T* GetDefaultObject() const
    {
        return static_cast<const T*>(mDefaultObject);
    }

    /**
     * Write an object of this type to a config value.
     *
     * @param   object              Pointer to a source object of "this" type.
     * @param   config,outValue     Target config value to write.
     * @return  True on success.
     */
    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const = 0;

    /**
     * Write an object of this type to binary data stream.
     *
     * @param   object              Pointer to a source object of "this" type.
     * @param   stream              Ouput data stream.
     * @return  True on success.
     */
    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const = 0;

    /**
     * Read an object of this type from a config value.
     *
     * @param   outObject           Pointer to a target object of "this" type.
     * @param   config,value        Source config value.
     * @return  True on success.
     */
    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, SerializationContext& context) const = 0;

    /**
     * Read an object of this type from binary data stream.
     *
     * @param   outObject           Pointer to a target object of "this" type.
     * @param   stream              Input data stream.
     * @return  True on success.
     */
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, SerializationContext& context) const = 0;

    /**
     * Deep compare two objects. Returns true if objecs are the same.
     */
    virtual bool Compare(const void* objectA, const void* objectB) const = 0;

    /**
     * Deep clone an object. Returns true if all fields were copied successfully.
     * Copy may fail for non-copyable-constructible types.
     */
    virtual bool Clone(void* destObject, const void* sourceObject) const = 0;

    /**
     * Try to copy data from a different type.
     * Mainly used in deserialization for upgrading compatible types.
     */
    virtual bool TryLoadFromDifferentType(void* outObject, const Variant& otherObject) const;

    // Returns true if can be copied/serialized via simple memcopy
    // Applies to fundamental types and POD structures
    virtual bool CanBeMemcopied() const;

    /**
     * Get member pointer by path.
     * Returns member pointer and type.
     * Returns false if traversal fails (e.g. unknown member name, array out of range, etc.)
     */
    virtual bool GetMemberByPath(void* object, const MemberPath& path, const Type*& outMemberType, void*& outMemberData) const;

protected:

    // type name (including namespace)
    Common::String mName;

    uint32 mSize;
    uint32 mAlignment;

    ConstructorFunc mConstructor;
    DestructorFunc mDestructor;

    TypeKind mKind;

    const void* mDefaultObject;

    virtual void OnInitialize(const TypeInfo& info);

};


} // namespace RTTI
} // namespace NFE
