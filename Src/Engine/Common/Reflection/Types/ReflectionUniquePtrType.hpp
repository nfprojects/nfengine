/**
 * @file
 * @author Witek902
 * @brief  Definition of UniquePtrType.
 */

#pragma once

#include "ReflectionPointerType.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Containers/UniquePtr.hpp"

namespace NFE {
namespace RTTI {


// base class for all unique pointer types
class NFCOMMON_API UniquePtrType : public PointerType
{
public:
    UniquePtrType(const Type* underlyingType);

    virtual void* GetPointedData(const void* uniquePtrObject) const override;
    virtual const Type* GetPointedDataType(const void* uniquePtrObject) const override;
    virtual void* Reset(void* uniquePtrObject, const Type* newDataType = nullptr) const override;
    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override;
    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, const SerializationContext& context) const override;
    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override final;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, const SerializationContext& context) const override final;
};

/**
 * Generic type creator for UniquePtr<T> types.
 */
template<typename T>
class TypeCreator<Common::UniquePtr<T>>
{
public:
    using TypeClass = UniquePtrType;
    using ObjectType = Common::UniquePtr<T>;

    static Type* CreateType()
    {
        return new UniquePtrType(ResolveType<T>());
    }

    static void InitializeType(Type* type)
    {
        const Type* underlyingType = ResolveType<T>();

        const Common::String typeName = Common::String("NFE::Common::UniquePtr<") + underlyingType->GetName() + '>';

        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::UniquePtr;
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
