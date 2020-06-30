/**
 * @file
 * @brief  Definition of SharedPtrType.
 */

#pragma once

#include "ReflectionPointerType.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Containers/SharedPtr.hpp"

namespace NFE {
namespace RTTI {


// base class for all shared pointer types
class NFCOMMON_API SharedPtrType : public PointerType
{
public:
    SharedPtrType(const Type* underlyingType);

    static const Common::String BuildTypeName(const Type* underlyingType);

    virtual void OnInitialize(const TypeInfo& info) override;

    // reset with an existing IObject
    void Assign(void* sharedPtrObject, const Common::SharedPtr<IObject>& newPtr) const;
    
    virtual void* GetPointedData(const void* ptrObject) const override;
    virtual const Type* GetPointedDataType(const void* ptrObject) const override;
    virtual void* Reset(void* ptrObject, const Type* newDataType = nullptr) const override;
    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override final;
    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, SerializationContext& context) const override final;
    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override final;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, SerializationContext& context) const override final;
};


/**
 * Generic type creator for SharedPtr<T> types.
 */
template<typename T>
class TypeCreator<Common::SharedPtr<T>>
{
public:
    static_assert(std::is_base_of_v<IObject, T>, "Reflection: SharedPtr must point to IObject-based class");

    using TypeClass = SharedPtrType;
    using ObjectType = Common::SharedPtr<T>;

    static Type* CreateType()
    {
        return new SharedPtrType(ResolveType<T>());
    }

    static void InitializeType(Type* type)
    {
        const Type* underlyingType = ResolveType<T>();

        const Common::String typeName = SharedPtrType::BuildTypeName(underlyingType);

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::SharedPtr;
        typeInfo.typeNameID = TypeNameID::SharedPtr;
        typeInfo.size = sizeof(ObjectType);
        typeInfo.alignment = alignof(ObjectType);
        typeInfo.name = typeName.Str();
        typeInfo.constructor = GetObjectConstructor<T>();
        typeInfo.destructor = GetObjectDestructor<T>();

        type->Initialize(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
