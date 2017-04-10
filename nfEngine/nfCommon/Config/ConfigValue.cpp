/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config value definition.
 */

#include "PCH.hpp"
#include "Config.hpp"
#include "ConfigValue.hpp"
#include "ConfigTokenizer.hpp"
#include "Logger/Logger.hpp"

#include <sstream>
#include <string.h>


namespace NFE {
namespace Common {

ConfigGenericValue::ConfigGenericValue(const Config* config)
    : ConfigValue(), mConfig(config)
{
    type = Type::Object;
    object = mConfig->GetRootNode();
}

ConfigGenericValue::ConfigGenericValue(const Config* config, const ConfigValue& val)
    : ConfigValue(val), mConfig(config)
{
}

bool ConfigGenericValue::HasMember(const char* key) const
{
    if (!IsObject())
        return false;

    bool found = false;
    auto callback = [&] (const char* iteratorKey, const ConfigValue&)
    {
        if (strcmp(iteratorKey, key) == 0)
        {
            found = true;
            return false;
        }
        return true;
    };

    mConfig->Iterate(callback, GetObj());
    return found;
}

ConfigGenericValue ConfigGenericValue::operator[](const char* key) const
{
    if (!IsObject())
        return ConfigGenericValue();

    ConfigValue valueFound;
    auto callback = [&] (const char* iteratorKey, const ConfigValue& value)
    {
        if (strcmp(iteratorKey, key) == 0)
        {
            valueFound = value;
            return false;
        }
        return true;
    };

    mConfig->Iterate(callback, GetObj());
    return ConfigGenericValue(mConfig, valueFound);
}

uint32 ConfigGenericValue::GetSize() const
{
    if (!IsArray())
        return 0;

    uint32 arraySize = 0;
    auto callback = [&] (uint32, const ConfigValue&)
    {
        arraySize++;
        return true;
    };

    mConfig->IterateArray(callback, GetArray());
    return arraySize;
}

ConfigGenericValue ConfigGenericValue::operator[](uint32 index) const
{
    if (!IsArray())
        return ConfigGenericValue();

    ConfigValue valueFound;
    auto callback = [&] (uint32 iteratorIndex, const ConfigValue& value)
    {
        if (iteratorIndex == index)
        {
            valueFound = value;
            return false;
        }
        return true;
    };

    mConfig->IterateArray(callback, GetArray());
    return ConfigGenericValue(mConfig, valueFound);
}

} // namespace Common
} // namespace NFE
