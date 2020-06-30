/**
 * @file
 * @author Witek902
 * @brief  Declaration of reflection system's StringType.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionTypeRegistry.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Containers/String.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for NFE::Common::String type
 */
class NFCOMMON_API StringType final : public Type
{
    NFE_MAKE_NONCOPYABLE(StringType)

public:
    StringType();
    ~StringType();

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
class TypeCreator<NFE::Common::String>
{
public:
    using TypeClass = StringType;

    static Type* CreateType()
    {
        return new StringType;
    }

    static void InitializeType(Type* type)
    {
        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::String;
        typeInfo.typeNameID = TypeNameID::String;
        typeInfo.size = sizeof(Common::String);
        typeInfo.alignment = alignof(Common::String);
        typeInfo.name = "String";
        typeInfo.constructor = GetObjectConstructor<Common::String>();
        typeInfo.destructor = GetObjectDestructor<Common::String>();

        type->Initialize(typeInfo);
    }
};


} // namespace RTTI
} // namespace NFE
