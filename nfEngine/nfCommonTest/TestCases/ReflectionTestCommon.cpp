#include "PCH.hpp"
#include "nfCommon/Reflection/ReflectionType.hpp"
#include "nfCommon/Config/Config.hpp"
#include "nfCommon/Config/ConfigValue.hpp"


using namespace NFE;
using namespace NFE::RTTI;

namespace helper {

void SerializeObject(const Type* type, const void* object, std::string& outString)
{
    Common::Config config;
    Common::ConfigValue value;
    ASSERT_TRUE(type->Serialize(object, config, value));
    Common::ConfigObject root;
    config.AddValue(root, "obj", value);
    config.SetRoot(root);

    outString = config.ToString(false);
}

void DeserializeObject(const Type* type, void* outObject, const std::string& string)
{
    Common::Config config;
    config.Parse(string.c_str());
    Common::ConfigGenericValue genericValue(&config);
    Common::ConfigValue value = genericValue["obj"];

    ASSERT_TRUE(type->Deserialize(outObject, config, value));
}

} // namespace helper
