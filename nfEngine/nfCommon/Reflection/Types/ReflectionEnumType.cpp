/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's EnumType class.
 */

#include "PCH.hpp"
#include "ReflectionEnumType.hpp"

#include "../../System/Assertion.hpp"
#include "../../Memory/DefaultAllocator.hpp"
#include "../../Logger/Logger.hpp"
#include "../../Config/Config.hpp"
#include "../../Config/ConfigValue.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

EnumType::EnumType(const EnumTypeInfo& info)
    : Type(info)
    , mOptions(info.options)
{
    NFE_ASSERT(GetSize() == 1 || GetSize() == 2 || GetSize() == 4 || GetSize() == 8, "Invalid enum type size");
    NFE_ASSERT(mOptions.Size() > 0, "Enum type must have at least one option");

    for (uint32 i = 0; i < mOptions.Size(); ++i)
    {
        for (uint32 j = i + 1; j < mOptions.Size(); ++j)
        {
            NFE_ASSERT(strcmp(mOptions[i].name, mOptions[j].name) != 0,
                       "Duplicated '%s' enum option in type '%s'. Fix enum type definition macros.",
                       mOptions[i].name, GetName());

            NFE_ASSERT(mOptions[i].value != mOptions[j].value,
                       "'%s' and '%s' enum options in type '%s' have the same value. This is not supported by the reflection system.",
                       mOptions[i].name, mOptions[j].name, GetName());
        }
    }
}

void EnumType::PrintInfo() const
{
    Type::PrintInfo();

#ifdef _DEBUG
    for (const EnumOption& option : mOptions)
    {
        LOG_DEBUG("    option '%s'='%llu'", option.name, option.value);
    }
#endif // _DEBUG
}

const EnumOptions& EnumType::GetOptions() const
{
    return mOptions;
}

bool EnumType::Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const
{
    // no need to access Config object itself
    NFE_UNUSED(config);

    uint64 integerValue = 0;
    switch (GetSize())
    {
    case 1:
        integerValue = *static_cast<const uint8*>(object);
        break;
    case 2:
        integerValue = *static_cast<const uint16*>(object);
        break;
    case 4:
        integerValue = *static_cast<const uint32*>(object);
        break;
    case 8:
        integerValue = *static_cast<const uint64*>(object);
        break;
    default:
        NFE_ASSERT(false, "Invalid enum type size");
        return false;
    }

    // TODO introduce hash map / binary search if the lookup is too slow
    const char* enumOptionName = nullptr;
    for (const EnumOption& option : mOptions)
    {
        if (option.value == integerValue)
        {
            enumOptionName = option.name;
            break;
        }
    }

    if (enumOptionName)
    {
        outValue = Common::ConfigValue(enumOptionName);
        return true;
    }

    LOG_ERROR("Could not find enum option for type '%s', value=%llu", GetName(), integerValue);
    return false;
}

bool EnumType::Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const
{
    NFE_UNUSED(config);

    if (value.Is<const char*>())
    {
        const char* optionName = value.Get<const char*>();
        uint64 integerValue = 0;
        bool enumOptionFound = false;
        // TODO introduce hash map / binary search if the lookup is too slow
        for (const EnumOption& option : mOptions)
        {
            if (strcmp(option.name, optionName) == 0)
            {
                integerValue = option.value;
                enumOptionFound = true;
                break;
            }
        }

        if (!enumOptionFound)
        {
            LOG_ERROR("Could not find enum option for type '%s', name=%llu", GetName(), optionName);
            return false;
        }

        switch (GetSize())
        {
        case 1:
            *static_cast<uint8*>(outObject) = static_cast<uint8>(integerValue);
            break;
        case 2:
            *static_cast<uint16*>(outObject) = static_cast<uint16>(integerValue);
            break;
        case 4:
            *static_cast<uint32*>(outObject) = static_cast<uint32>(integerValue);
            break;
        case 8:
            *static_cast<uint64*>(outObject) = static_cast<uint64>(integerValue);
            break;
        default:
            NFE_ASSERT(false, "Invalid enum type size");
            return false;
        }

        return true;
    }

    LOG_ERROR("Expected string in the config");
    return false;
}

} // namespace RTTI
} // namespace NFE
