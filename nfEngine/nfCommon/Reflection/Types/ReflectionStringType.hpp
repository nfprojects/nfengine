/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's StringType.
 */

#pragma once

#include "../../nfCommon.hpp"
#include "ReflectionType.hpp"
#include "../ReflectionTypeRegistry.hpp"
#include "../../Containers/String.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for NFE::Common::String type
 */
class NFCOMMON_API StringType : public Type
{
    NFE_MAKE_NONCOPYABLE(StringType)

public:
    StringType(const TypeInfo& info);
    ~StringType();

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override;
    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override;
};


/**
 * Type creator for "String" type
 */
template<>
class TypeCreator<NFE::Common::String>
{
public:
    using TypeClass = StringType;

    static TypePtr CreateType()
    {
        TypeInfo typeInfo;
        typeInfo.kind = TypeKind::Fundamental;
        typeInfo.size = sizeof(Common::String);
        typeInfo.alignment = alignof(Common::String);
        typeInfo.name = "NFE::Common::String";
        typeInfo.constructor = []() { return new Common::String; };
        typeInfo.arrayConstructor = [](uint32 num) { return new Common::String[num]; };

        return TypePtr(new StringType(typeInfo));
    }
};


} // namespace RTTI
} // namespace NFE
