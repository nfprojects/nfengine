/**
 * @file
 * @author Witek902
 * @brief  Definition of StaticArrayType.
 */

#pragma once

#include "ReflectionResizableArrayType.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Containers/StaticArray.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for StaticArray<T,N> types.
 */
class NFCOMMON_API StaticArrayType final : public IResizableArrayType
{
    NFE_MAKE_NONCOPYABLE(StaticArrayType)

public:
    StaticArrayType(const Type* underlyingType, uint32 capacity);

    static const Common::String BuildTypeName(const Type* underlyingType, uint32 capacity);

    // ResizableArrayType interface implementation
    virtual uint32 GetArraySize(const void* arrayObject) const override final;
    virtual uint32 GetMaxCapacity() const override final;
    virtual bool ResizeArray(void* arrayObject, uint32 targetSize) const override final;
    virtual void* GetElementPointer(void* arrayObject, uint32 index) const override;
    virtual const void* GetElementPointer(const void* arrayObject, uint32 index) const override;

    // Type interface implementation
    virtual void PrintInfo() const override final;

    /*
    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override final;
    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, SerializationContext& context) const override final;
    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override final;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, SerializationContext& context) const override final;
    virtual bool TryLoadFromDifferentType(void* outObject, const Variant& otherObject) const override final;
    virtual bool Compare(const void* objectA, const void* objectB) const override final;
    virtual bool Clone(void* destObject, const void* sourceObject) const override final;
    */

private:
    uint32 mCapacity;
};


/**
 * Generic type creator for StaticArray<T,N> types.
 */
template<typename T, uint32 Capacity>
class TypeCreator<Common::StaticArray<T,Capacity>>
{
public:
    using TypeClass = StaticArrayType;
    using TypeInfoClass = TypeInfo;
    using ObjectType = Common::StaticArray<T,Capacity>;

    static Type* CreateType()
    {
        const Type* underlyingType = ResolveType<T>();
        return new StaticArrayType(underlyingType, Capacity);
    }

    static void InitializeType(Type* type)
    {
        const Type* arrayElementType = ResolveType<T>();

        const Common::String typeName = StaticArrayType::BuildTypeName(arrayElementType, Capacity);

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::StaticArray;
        typeInfo.typeNameID = TypeNameID::StaticArray;
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
