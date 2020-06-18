/**
 * @file
 * @author Witek902
 * @brief  Definition of DynArrayPtr.
 */

#pragma once

#include "ReflectionArrayType.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Containers/DynArray.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for DynArray<T> types.
 */
class NFCOMMON_API DynArrayType : public ArrayType
{
    NFE_MAKE_NONCOPYABLE(DynArrayType)

public:
    NFE_FORCE_INLINE DynArrayType(const Type* underlyingType)
        : ArrayType(underlyingType)
    { }

    // get number of array elements
    uint32 GetArraySize(const void* arrayObject) const;

    // resize array object
    virtual bool ResizeArray(void* arrayObject, uint32 targetSize) const = 0;

    // Type interface implementation
    virtual void PrintInfo() const override final;
    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override final;
    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, const SerializationContext& context) const override final;
    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override final;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, const SerializationContext& context) const override final;
    virtual bool Compare(const void* objectA, const void* objectB) const override final;
    virtual bool Clone(void* destObject, const void* sourceObject) const override final;
};

/**
 * Specialized type information for DynArray<T> types.
 */
template<typename T>
class DynArrayTypeImpl final : public DynArrayType
{
    NFE_MAKE_NONCOPYABLE(DynArrayTypeImpl)

public:
    using ObjectType = Common::DynArray<T>;

    NFE_FORCE_INLINE DynArrayTypeImpl()
        : DynArrayType(GetType<T>())
    { }

    virtual bool ResizeArray(void* arrayObject, uint32 targetSize) const override
    {
        NFE_ASSERT(arrayObject, "Invalid array object");

        ObjectType& typedObject = *static_cast<ObjectType*>(arrayObject);
        return typedObject.Resize(targetSize);
    }

    virtual void* GetElementPointer(void* arrayObject, uint32 index) const override
    {
        NFE_ASSERT(arrayObject, "Invalid array object");

        ObjectType& typedObject = *static_cast<ObjectType*>(arrayObject);
        return &typedObject[index];
    }

    virtual const void* GetElementPointer(const void* arrayObject, uint32 index) const override
    {
        NFE_ASSERT(arrayObject, "Invalid array object");

        const ObjectType& typedObject = *static_cast<const ObjectType*>(arrayObject);
        return &typedObject[index];
    }
};


/**
 * Generic type creator for DynArray<T> types.
 */
template<typename T>
class TypeCreator<Common::DynArray<T>>
{
public:
    using TypeClass = DynArrayTypeImpl<T>;
    using TypeInfoClass = TypeInfo;
    using ObjectType = Common::DynArray<T>;

    static Type* CreateType()
    {
        return new DynArrayTypeImpl<T>();
    }

    static void InitializeType(Type* type)
    {
        const Type* templateArgumentType = ResolveType<T>();

        const Common::String typeName = Common::String("NFE::Common::DynArray<") + templateArgumentType->GetName() + '>';

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::DynArray;
        typeInfo.size = sizeof(ObjectType);
        typeInfo.alignment = alignof(ObjectType);
        typeInfo.name = typeName.Str();
        typeInfo.constructor = GetObjectConstructor<ObjectType>();
        typeInfo.destructor = GetObjectDestructor<ObjectType>();

        type->Initialize(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
