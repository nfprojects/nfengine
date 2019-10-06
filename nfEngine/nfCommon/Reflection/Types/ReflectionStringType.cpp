/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of reflection system's StringType.
 */

#include "PCH.hpp"
#include "ReflectionStringType.hpp"
#include "../../Config/Config.hpp"


namespace NFE {
namespace RTTI {

StringType::StringType(const TypeInfo& info)
    : Type(info)
{ }

StringType::~StringType() = default;

bool StringType::Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const
{
    // no need to access Config object itself, all the data for fundamental types is contained in ConfigValue
    NFE_UNUSED(config);

    const Common::String* typedObject = static_cast<const Common::String*>(object);
    outValue = Common::ConfigValue(typedObject->Str());
    return true;
}

bool StringType::Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const
{
    NFE_UNUSED(config);

    if (value.Is<const char*>())
    {
        Common::String* typedObject = static_cast<Common::String*>(outObject);
        *typedObject = value.Get<const char*>();
        return true;
    }

    NFE_LOG_ERROR("Expected string in the config");
    return false;
}

bool StringType::Compare(const void* objectA, const void* objectB) const
{
    const Common::String& typedObjectA = *static_cast<const Common::String*>(objectA);
    const Common::String& typedObjectB = *static_cast<const Common::String*>(objectB);
    return typedObjectA == typedObjectB;
}

} // namespace RTTI
} // namespace NFE
