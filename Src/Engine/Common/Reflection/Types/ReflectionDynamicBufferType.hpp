/**
 * @file
 * @author Witek902
 * @brief  Declaration of reflection system's DynamicBufferType.
 */

#pragma once

#include "ReflectionStaticBufferType.hpp"
#include "../../Memory/DynamicBuffer.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for NFE::Common::DynamicBuffer type
 */
class NFCOMMON_API DynamicBufferType : public StaticBufferType
{
    NFE_MAKE_NONCOPYABLE(DynamicBufferType)

public:
    DynamicBufferType();
    ~DynamicBufferType();

    bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, SerializationContext& context) const override;
    bool DeserializeBinary(void* outObject, Common::InputStream& stream, SerializationContext& context) const override;
    bool Clone(void* destObject, const void* sourceObject) const override;
};


/**
 * Type creator for "String" type
 */
template<>
class TypeCreator<NFE::Common::DynamicBuffer>
{
public:
    using TypeClass = DynamicBufferType;

    static Type* CreateType()
    {
        return new DynamicBufferType;
    }

    static void InitializeType(Type* type)
    {
        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::DynamicBuffer;
        typeInfo.typeNameID = TypeNameID::Buffer;
        typeInfo.size = sizeof(Common::DynamicBuffer);
        typeInfo.alignment = alignof(Common::DynamicBuffer);
        typeInfo.name = "DynamicBuffer";
        typeInfo.constructor = GetObjectConstructor<Common::DynamicBuffer>();
        typeInfo.destructor = GetObjectDestructor<Common::DynamicBuffer>();

        type->Initialize(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
