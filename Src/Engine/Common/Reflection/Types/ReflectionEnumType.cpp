/**
 * @file
 * @author Witek902
 * @brief  Definitions of reflection system's EnumType class.
 */

#include "PCH.hpp"
#include "ReflectionEnumType.hpp"
#include "../SerializationContext.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


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
        NFE_ASSERT(!mOptions[i].name.Empty(), "Enum option cannot be empty");

        for (uint32 j = i + 1; j < mOptions.Size(); ++j)
        {
            NFE_ASSERT(mOptions[i].name != mOptions[j].name,
                "Duplicated '%.*s' enum option in type '%s'. Fix enum type definition macros.",
                mOptions[i].name.Length(), mOptions[i].name.Data(),
                GetName().Str());

            NFE_ASSERT(mOptions[i].value != mOptions[j].value,
                "'%.*s' and '%.*s' enum options in type '%.*s' have the same value (%llu). This is not supported by the reflection system.",
                mOptions[i].name.Length(), mOptions[i].name.Data(),
                mOptions[j].name.Length(), mOptions[j].name.Data(),
                GetName().Str(),
                mOptions[i].value);
        }
    }
}

EnumType::~EnumType() = default;

void EnumType::PrintInfo() const
{
    Type::PrintInfo();

#ifdef _DEBUG
    for (const EnumOption& option : mOptions)
    {
        NFE_LOG_DEBUG("    option '%.*s'=%llu", option.name.Length(), option.name.Data(), option.value);
    }
#endif // _DEBUG
}

const EnumOptions& EnumType::GetOptions() const
{
    return mOptions;
}

bool EnumType::Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const
{
    // no need to access Config object itself
    NFE_UNUSED(config);
    NFE_UNUSED(context);

    const uint64 integerValue = ReadRawValue(object);
    const StringView enumOptionName = FindOptionByValue(integerValue);

    if (!enumOptionName.Empty())
    {
        outValue = Common::ConfigValue(enumOptionName);
        return true;
    }

    NFE_LOG_ERROR("Could not find enum option for type '%s', value=%llu", GetName().Str(), integerValue);
    return false;
}

bool EnumType::Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, const SerializationContext& context) const
{
    NFE_UNUSED(config);
    NFE_UNUSED(context);

    if (value.Is<const char*>())
    {
        const char* optionName = value.Get<const char*>();

        uint64 integerValue = 0;
        if (!FindOptionByName(StringView(optionName), integerValue))
        {
            // TODO report missing enum
            NFE_LOG_ERROR("Could not find enum option for type '%s', name=%s", GetName().Str(), optionName);
            return false;
        }

        WriteRawValue(outObject, integerValue);
        return true;
    }

    // TODO report type mismatch
    NFE_LOG_ERROR("Expected string in the config");
    return false;
}

bool EnumType::SerializeBinary(const void* object, OutputStream* stream, SerializationContext& context) const
{
    // always serialize enums as strings

    const uint64 integerValue = ReadRawValue(object);
    const StringView enumOptionName = FindOptionByValue(integerValue);

    if (enumOptionName.Empty())
    {
        NFE_LOG_ERROR("Could not find enum option for type '%s', value=%llu", GetName().Str(), integerValue);
        return false;
    }

    uint32 strIndex = context.MapString(enumOptionName);
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

bool EnumType::DeserializeBinary(void* outObject, InputStream& stream, const SerializationContext& context) const
{
    uint32 strIndex;
    if (!stream.ReadCompressedUint(strIndex))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return false;
    }
    StringView enumOptionName;
    if (!context.UnmapString(strIndex, enumOptionName))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return false;
    }

    uint64 integerValue = 0;
    if (!FindOptionByName(enumOptionName, integerValue))
    {
        // TODO report missing enum
        NFE_LOG_ERROR("Could not find enum option for type '%s', name=%s", GetName().Str(), enumOptionName.Length(), enumOptionName.Data());
        return false;
    }

    WriteRawValue(outObject, integerValue);
    return true;
}

bool EnumType::Compare(const void* objectA, const void* objectB) const
{
    return ReadRawValue(objectA) == ReadRawValue(objectB);
}

bool EnumType::Clone(void* destObject, const void* sourceObject) const
{
    WriteRawValue(destObject, ReadRawValue(sourceObject));
    return true;
}

const StringView EnumType::FindOptionByValue(uint64 value) const
{
    // TODO introduce hash map / binary search if the lookup is too slow

    for (const EnumOption& option : mOptions)
    {
        if (option.value == value)
        {
            return option.name;
        }
    }

    return StringView();
}

bool EnumType::FindOptionByName(const Common::StringView name, uint64& outValue) const
{
    // TODO introduce hash map / binary search if the lookup is too slow

    for (const EnumOption& option : mOptions)
    {
        if (name == option.name)
        {
            outValue = option.value;
            return true;
        }
    }

    return false;
}

void EnumType::WriteRawValue(void* object, uint64 rawValue) const
{
    NFE_ASSERT(object, "Invalid object");

    const size_t size = GetSize();
    if (size == 1u)
    {
        uint8* typedObject = reinterpret_cast<uint8*>(object);
        *typedObject = static_cast<uint8>(rawValue);
    }
    else if (size == 2u)
    {
        uint16* typedObject = reinterpret_cast<uint16*>(object);
        *typedObject = static_cast<uint16>(rawValue);
    }
    else if (size == 4u)
    {
        uint32* typedObject = reinterpret_cast<uint32*>(object);
        *typedObject = static_cast<uint32>(rawValue);
    }
    else if (size == 8u)
    {
        uint64* typedObject = reinterpret_cast<uint64*>(object);
        *typedObject = static_cast<uint64>(rawValue);
    }
    else
    {
        NFE_FATAL("Invalid enum type size: %zu", size);
    }
}

uint64 EnumType::ReadRawValue(const void* object) const
{
    NFE_ASSERT(object, "Invalid object");

    const size_t size = GetSize();
    if (size == 1u)
    {
        return *reinterpret_cast<const uint8*>(object);
    }
    else if (size == 2u)
    {
        return *reinterpret_cast<const uint16*>(object);
    }
    else if (size == 4u)
    {
        return *reinterpret_cast<const uint32*>(object);
    }
    else if (size == 8u)
    {
        return *reinterpret_cast<const uint64*>(object);
    }
    else
    {
        NFE_FATAL("Invalid enum type size: %zu", size);
    }

    return UINT64_MAX;
}

} // namespace RTTI
} // namespace NFE
