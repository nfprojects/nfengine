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
    String,             // NFE::String type
    NativeArray,        // T[N] types
    Array,              // NFE::DynArray<T>, NFE::StaticArray<T,N> types
    UniquePtr,          // NFE::UniquePtr<T> types
    SharedPtr,          // NFE::SharedPtr<T> types
    Class,              // class type

    // TODO bitfields
};

// DO NOT CHANGE EXISTING IDs - IT WOULD BREAK THE SERIALIZATION !!!
enum class TypeNameID : uint8
{
    Invalid             = 0x0,
    Fundamental_Bool    = 0x1,
    Fundamental_Uint8   = 0x2,
    Fundamental_Uint16  = 0x3,
    Fundamental_Uint32  = 0x4,
    Fundamental_Uint64  = 0x5,
    Fundamental_Int8    = 0x6,
    Fundamental_Int16   = 0x7,
    Fundamental_Int32   = 0x8,
    Fundamental_Int64   = 0x9,
    Fundamental_Float   = 0xA,
    Fundamental_Double  = 0xB,
    Class               = 0x10,
    Enumeration         = 0x11,
    NativeArray         = 0x12,
    Array               = 0x13,
    UniquePtr           = 0x15,
    SharedPtr           = 0x16,
    String              = 0x17,
};

using ConstructorFunc = std::function<void(void*)>;
using DestructorFunc = std::function<void(void*)>;

/**
 * Instead of filling Type class directly, this class is used as a helper.
 */
struct TypeInfo
{
public:
    const char* name = nullptr;
    size_t size = 0u;
    size_t alignment = 0u;
    bool dynamicType = false;
    TypeKind kind = TypeKind::Undefined;
    TypeNameID typeNameID = TypeNameID::Invalid;
    ConstructorFunc constructor;
    DestructorFunc destructor;
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

    // Get type kind
    NFE_FORCE_INLINE TypeKind GetKind() const { return mKind; }

    // Check if the type is dynamic
    NFE_FORCE_INLINE bool IsDynamicType() const { return mIsDynamicType; }

    // Check if the type is enabled (active)
    NFE_FORCE_INLINE bool IsEnabled() const { return mIsEnabled; }

    // Get type name ID
    NFE_FORCE_INLINE TypeNameID GetTypeNameID() const { return mTypeNameID; }

    // Can be constructed (without arguments)?
    NFE_FORCE_INLINE bool IsConstructible() const { return mConstructor != nullptr; }

    // Enable/disable
    NFE_FORCE_INLINE void UNIT_TEST_ONLY_Enable(bool enableState) { mIsEnabled = enableState; }

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

    // Write type name in binary form
    virtual bool SerializeTypeName(Common::OutputStream* stream, SerializationContext& context) const;

protected:

    // type name (including namespace)
    Common::String mName;

    uint32 mSize;
    uint32 mAlignment;

    // Dynamic types are created during deserialization for types that are not fully known at the runtime.
    // For example, there could be "UniquePtr<int32>" type object created for that type defined in the code,
    // but in the data there's an unknown (unregistered) type "UniquePtr<int8>".
    // This will happen when a property of type "UniquePtr<int8>" was changed in the code into "UniquePtr<int32>",
    // but the serialized data remained the same and is now being deserialized (using the new code).
    // In this case, the type object for type "UniquePtr<int8>" will be created at the runtime, thus it's "dynamic".
    bool mIsDynamicType : 1;

    // For unit test purposes only
    bool mIsEnabled : 1;

    ConstructorFunc mConstructor;
    DestructorFunc mDestructor;

    TypeKind mKind;
    TypeNameID mTypeNameID;

    const void* mDefaultObject;

    virtual void OnInitialize(const TypeInfo& info);

};


} // namespace RTTI
} // namespace NFE
