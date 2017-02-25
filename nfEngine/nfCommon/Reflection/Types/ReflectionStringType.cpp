/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definition of reflection system's StringType.
 */

#include "PCH.hpp"
#include "ReflectionStringType.hpp"
#include "../../Logger/Logger.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {


bool StringType::Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const
{
    // no need to access Config object itself, all the data for fundamental types is contained in ConfigValue
    UNUSED(config);

    const Common::String* typedObject = static_cast<const Common::String*>(object);
    outValue = Common::ConfigValue(typedObject->Str());
    return true;
}

bool StringType::Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const
{
    UNUSED(config);

    if (value.Is<const char*>())
    {
        Common::String* typedObject = static_cast<Common::String*>(outObject);
        *typedObject = value.Get<const char*>();
        return true;
    }

    LOG_ERROR("Expected string in the config");
    return false;
}


} // namespace RTTI
} // namespace NFE
