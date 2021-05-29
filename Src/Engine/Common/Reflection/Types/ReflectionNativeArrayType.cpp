/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionNativeArrayType.hpp"
#include "../SerializationContext.hpp"
#include "../ReflectionVariant.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"

namespace NFE {
namespace RTTI {

using namespace Common;

const String NativeArrayType::BuildTypeName(const Type* underlyingType, uint32 arraySize)
{
    NFE_ASSERT(underlyingType, "Invalid type");
    return String::Printf("%s[%u]", underlyingType->GetName().Str(), arraySize);
}

NativeArrayType::NativeArrayType(uint32 arraySize, const Type* elementType)
    : IArrayType(elementType)
    , mArraySize(arraySize)
{
    NFE_ASSERT(mArraySize > 0, "Native array size cannot be empty. Type: %s", GetName().Str());
}

void NativeArrayType::OnInitialize(const TypeInfo& info)
{
    NFE_ASSERT(info.size == mArraySize * mUnderlyingType->GetSize(), "Invalid array size");
    NFE_ASSERT(info.alignment == mUnderlyingType->GetAlignment(), "Invalid array alignment");

    IArrayType::OnInitialize(info);

    mConstructor = [this] (void* object)
    {
        for (uint32 i = 0; i < mArraySize; ++i)
        {
            mUnderlyingType->ConstructObject(GetElementPointer(object, i));
        }
    };

    mDestructor = [this] (void* object)
    {
        for (uint32 i = 0; i < mArraySize; ++i)
        {
            mUnderlyingType->DestructObject(GetElementPointer(object, i));
        }
    };
}

void NativeArrayType::PrintInfo() const
{
    Type::PrintInfo();
    NFE_LOG_DEBUG("  array size = %u", mArraySize);
}

uint32 NativeArrayType::GetArraySize(const void* arrayObject) const
{
    NFE_ASSERT(arrayObject, "Invalid array object");
    NFE_UNUSED(arrayObject);
    return mArraySize;
}

void* NativeArrayType::GetElementPointer(void* arrayData, uint32 index) const
{
    NFE_ASSERT(index < mArraySize, "Invalid array index %u, array size is %u", index, mArraySize);
    return reinterpret_cast<uint8*>(arrayData) + index * GetUnderlyingType()->GetSize();
}

const void* NativeArrayType::GetElementPointer(const void* arrayData, uint32 index) const
{
    NFE_ASSERT(index < mArraySize, "Invalid array index %u, array size is %u", index, mArraySize);
    return reinterpret_cast<const uint8*>(arrayData) + index * GetUnderlyingType()->GetSize();
}

bool NativeArrayType::Compare(const void* objectA, const void* objectB) const
{
    const Type* elementType = GetUnderlyingType();

    if (elementType->CanBeMemcopied())
    {
        NFE_ASSERT(GetElementPointer(objectA, 0u) == objectA, "Array pointer is expected to point at first element");
        NFE_ASSERT(GetElementPointer(objectB, 0u) == objectB, "Array pointer is expected to point at first element");

        const size_t elementSize = elementType->GetSize();
        const size_t numBytesInArray = elementSize * mArraySize;
        return 0 == memcmp(objectA, objectB, numBytesInArray);
    }

    for (uint32 i = 0; i < mArraySize; ++i)
    {
        if (!elementType->Compare(GetElementPointer(objectA, i), GetElementPointer(objectB, i)))
        {
            return false;
        }
    }

    return true;
}

bool NativeArrayType::Clone(void* destObject, const void* sourceObject) const
{
    const Type* elementType = GetUnderlyingType();

    if (elementType->CanBeMemcopied())
    {
        NFE_ASSERT(GetElementPointer(destObject, 0u) == destObject, "Array pointer is expected to point at first element");
        NFE_ASSERT(GetElementPointer(sourceObject, 0u) == sourceObject, "Array pointer is expected to point at first element");

        const size_t elementSize = elementType->GetSize();
        const size_t numBytesToCopy = elementSize * mArraySize;
        memcpy(destObject, sourceObject, numBytesToCopy);
        return true;
    }

    bool success = true;
    for (uint32 i = 0; i < mArraySize; ++i)
    {
        if (!elementType->Clone(GetElementPointer(destObject, i), GetElementPointer(sourceObject, i)))
        {
            success = false;
        }
    }

    return success;
}

bool NativeArrayType::TryLoadFromDifferentType(void* outObject, const VariantView& otherObject) const
{
    NFE_ASSERT(otherObject.GetType(), "Empty variant");

    if (otherObject.GetType()->GetKind() == TypeKind::NativeArray)
    {
        const NativeArrayType* otherType = static_cast<const NativeArrayType*>(otherObject.GetType());
        const Type* otherUnderlyingType = otherType->GetUnderlyingType();

        // can only upgrade from smaller array (otherwise we may loose data)
        if (otherType->GetArraySize() <= GetArraySize())
        {
            if (otherUnderlyingType == GetUnderlyingType())
            {
                // underlying type is matching - copy array elements directly
                return otherType->Clone(outObject, otherObject.GetData());
            }
            else
            {
                // underlying type are mismatched - try upgrading element by element
                for (uint32 i = 0; i < otherType->GetArraySize(); ++i)
                {
                    const void* otherElement = otherType->GetElementPointer(otherObject.GetData(), i);
                    const VariantView otherElementVariant(otherUnderlyingType, otherElement);

                    if (!GetUnderlyingType()->TryLoadFromDifferentType(GetElementPointer(outObject, i), otherElementVariant))
                    {
                        NFE_LOG_ERROR("Failed to upgrade array element (%u) from type '%s' to '%s'",
                            i, otherUnderlyingType->GetName().Str(), GetUnderlyingType()->GetName().Str());
                        return false;
                    }
                }

                return true;
            }
        }
    }
    else if (otherObject.GetType()->GetKind() == TypeKind::Array)
    {
        // TODO read when array size matches
    }

    return false;
}

bool NativeArrayType::CanBeMemcopied() const
{
    return mUnderlyingType->CanBeMemcopied();
}

bool NativeArrayType::Serialize(const void* object, IConfig& config, ConfigValue& outValue, SerializationContext& context) const
{
    using namespace Common;

    // extract serialization information
    const Type* elementType = GetUnderlyingType();

    // sanity checks
    NFE_ASSERT(elementType, "Invalid native array element type");
    NFE_ASSERT(object, "Trying to serialize nullptr");

    // serialize array elements
    ConfigArray configArray;
    for (uint32 i = 0; i < mArraySize; ++i)
    {
        ConfigValue arrayElementValue;
        const void* arrayElement = GetElementPointer(object, i);
        if (!elementType->Serialize(arrayElement, config, arrayElementValue, context))
        {
            NFE_LOG_ERROR("Failed to serialize native array element (index %u/%u)", i, mArraySize);
            return false;
        }
        config.AddValue(configArray, arrayElementValue);
    }

    // success
    outValue = configArray;
    return true;
}

bool NativeArrayType::Deserialize(void* outObject, const IConfig& config, const ConfigValue& value, SerializationContext& context) const
{
    using namespace Common;

    // extract serialization information
    const Type* elementType = GetUnderlyingType();

    // sanity checks
    NFE_ASSERT(elementType, "Invalid native array element type");
    NFE_ASSERT(outObject, "Trying to deserialize to nullptr");

    if (!value.IsArray())
    {
        NFE_LOG_ERROR("Expected array type");
        return false;
    }

    uint32 numDeserializedArrayElements = 0;
    auto arrayIteratorCallback = [&] (int index, const ConfigValue& configArrayElement)
    {
        if (static_cast<uint32>(index) >= mArraySize)
        {
            NFE_LOG_WARNING("Deserialized native array object has too many elements (%u expected). "
                "Elements will be dropped", mArraySize);
            return false;
        }

        void* outArrayElement = GetElementPointer(outObject, index);
        if (!elementType->Deserialize(outArrayElement, config, configArrayElement, context))
        {
            NFE_LOG_ERROR("Failed to parse native array element at index %i", index);
            return false;
        }

        numDeserializedArrayElements++;
        return true;
    };

    config.IterateArray(arrayIteratorCallback, value.GetArray());

    if (numDeserializedArrayElements != mArraySize)
    {
        NFE_LOG_WARNING("Deserialized native array object has too few array elements (%u found, %u expected).",
            numDeserializedArrayElements, mArraySize);
        // TODO initialize missing elements with default values (run default constructor)
    }

    return true;
}

bool NativeArrayType::SerializeBinary(const void* object, OutputStream* stream, SerializationContext& context) const
{
    const Type* elementType = GetUnderlyingType();
    const size_t elementSize = elementType->GetSize();
    const uint8* firstElementData = reinterpret_cast<const uint8*>(GetElementPointer(object, 0u));

    if (elementType->CanBeMemcopied())
    {
        if (!context.IsMapping()) // don't serialize anything in mapping stage
        {
            // bulk serialize whole array if can be memcopied
            size_t numBytesToWrite = elementSize * mArraySize;
            if (numBytesToWrite != stream->Write(firstElementData, numBytesToWrite))
            {
                return false;
            }
        }
    }
    else
    {
        for (uint32 i = 0; i < mArraySize; ++i)
        {
            if (!elementType->SerializeBinary(firstElementData + elementSize * i, stream, context))
            {
                return false;
            }
        }
    }

    return true;
}

bool NativeArrayType::DeserializeBinary(void* outObject, InputStream& stream, SerializationContext& context) const
{
    const Type* elementType = GetUnderlyingType();
    const size_t elementSize = elementType->GetSize();
    uint8* firstElementData = reinterpret_cast<uint8*>(GetElementPointer(outObject, 0u));

    if (elementType->CanBeMemcopied())
    {
        // bulk deserialize whole array if can be memcopied
        size_t numBytesToRead = elementSize * mArraySize;
        if (numBytesToRead != stream.Read(firstElementData, numBytesToRead))
        {
            return false;
        }
    }
    else
    {
        for (uint32 i = 0; i < mArraySize; ++i)
        {
            if (!elementType->DeserializeBinary(firstElementData + elementSize * i, stream, context))
            {
                return false;
            }
        }
    }

    return true;
}

bool NativeArrayType::SerializeTypeName(OutputStream* stream, SerializationContext& context) const
{
    // write header
    if (!Type::SerializeTypeName(stream, context))
    {
        return false;
    }

    // append array size
    if (stream)
    {
        if (!stream->WriteCompressedPositiveInt(mArraySize))
        {
            return false;
        }
    }

    // append inner type
    return mUnderlyingType->SerializeTypeName(stream, context);
}

} // namespace RTTI
} // namespace NFE
