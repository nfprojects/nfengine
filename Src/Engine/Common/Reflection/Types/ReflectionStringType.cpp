/**
 * @file
 * @author Witek902
 * @brief  Definition of reflection system's StringType.
 */

#include "PCH.hpp"
#include "ReflectionStringType.hpp"
#include "../SerializationContext.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

StringType::StringType() = default;

StringType::~StringType() = default;

bool StringType::Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const
{
    // no need to access Config object itself, all the data for fundamental types is contained in ConfigValue
    NFE_UNUSED(config);
    NFE_UNUSED(context);

    const Common::String* typedObject = static_cast<const Common::String*>(object);
    outValue = Common::ConfigValue(typedObject->Str());
    return true;
}

bool StringType::Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, const SerializationContext& context) const
{
    NFE_UNUSED(config);
    NFE_UNUSED(context);

    if (value.Is<const char*>())
    {
        Common::String* typedObject = static_cast<Common::String*>(outObject);
        *typedObject = value.Get<const char*>();
        return true;
    }

    // TODO type mismatch
    NFE_LOG_ERROR("Expected string in the config");
    return false;
}

bool StringType::SerializeBinary(const void* object, OutputStream* stream, SerializationContext& context) const
{
    const Common::String* typedObject = static_cast<const Common::String*>(object);

    // TODO if string is very long maybe it's better to serialize it directly

    uint32 strIndex = context.MapString(*typedObject);
    if (strIndex == SerializationContext::InvalidIndex)
    {
        return false;
    }

    if (context.IsMapping())
    {
        // don't serialize anything in mapping stage
        return true;
    }

    return stream->WriteCompressedUint(strIndex);
}

bool StringType::DeserializeBinary(void* outObject, InputStream& stream, const SerializationContext& context) const
{
    uint32 strIndex;
    if (!stream.ReadCompressedUint(strIndex))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return false;
    }
    StringView unmappedString;
    if (!context.UnmapString(strIndex, unmappedString))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return false;
    }

    Common::String* typedObject = static_cast<Common::String*>(outObject);
    *typedObject = unmappedString;

    return true;
}

bool StringType::Compare(const void* objectA, const void* objectB) const
{
    const Common::String& typedObjectA = *static_cast<const Common::String*>(objectA);
    const Common::String& typedObjectB = *static_cast<const Common::String*>(objectB);
    return typedObjectA == typedObjectB;
}

bool StringType::Clone(void* destObject, const void* sourceObject) const
{
    *static_cast<Common::String*>(destObject) = *static_cast<const Common::String*>(sourceObject);
    return true;
}

} // namespace RTTI
} // namespace NFE
