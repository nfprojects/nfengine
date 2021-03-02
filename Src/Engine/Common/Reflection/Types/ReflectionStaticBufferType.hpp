/**
 * @file
 * @author Witek902
 * @brief  Declaration of reflection system's StaticBufferType.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionTypeRegistry.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Memory/StaticBuffer.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for NFE::Common::StaticBuffer type
 */
class NFCOMMON_API StaticBufferType : public Type
{
    NFE_MAKE_NONCOPYABLE(StaticBufferType)

public:
    StaticBufferType();
    ~StaticBufferType();

    bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override;
    bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override;
    bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, SerializationContext& context) const override;
    bool DeserializeBinary(void* outObject, Common::InputStream& stream, SerializationContext& context) const override;
    bool Compare(const void* objectA, const void* objectB) const override;
    bool Clone(void* destObject, const void* sourceObject) const override;
};


/**
 * Type creator for "String" type
 */
template<>
class TypeCreator<NFE::Common::StaticBuffer>
{
public:
    using TypeClass = StaticBufferType;

    static Type* CreateType()
    {
        return new StaticBufferType;
    }

    static void InitializeType(Type* type)
    {
        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::StaticBuffer;
        typeInfo.typeNameID = TypeNameID::Buffer;
        typeInfo.size = sizeof(Common::StaticBuffer);
        typeInfo.alignment = alignof(Common::StaticBuffer);
        typeInfo.name = "StaticBuffer";
        typeInfo.constructor = GetObjectConstructor<Common::StaticBuffer>();
        typeInfo.destructor = GetObjectDestructor<Common::StaticBuffer>();

        type->Initialize(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
