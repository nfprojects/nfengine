/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionResizableArrayType.hpp"
#include "ReflectionNativeArrayType.hpp"
#include "../SerializationContext.hpp"
#include "../../Config/ConfigInterface.hpp"
#include "../../Utils/Stream/OutputStream.hpp"
#include "../../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

bool IResizableArrayType::TryLoadFromDifferentType(void* outObject, const Variant& otherObject) const
{
    NFE_ASSERT(otherObject.GetType(), "Empty variant");

    // only native array can be converted to DynArray
    if (otherObject.GetType()->GetKind() != TypeKind::NativeArray)
    {
        return false;
    }

    const NativeArrayType* otherType = static_cast<const NativeArrayType*>(otherObject.GetType());
    const Type* underlyingType = GetUnderlyingType();

    // TODO should be able to upgrade if underlying types are compatible as well
    if (otherType->GetUnderlyingType() != underlyingType)
    {
        return false;
    }

    const uint32 targetSize = otherType->GetArraySize();
    if (!ResizeArray(outObject, targetSize))
    {
        return false;
    }

    // copy array elements
    for (uint32 i = 0; i < targetSize; ++i)
    {
        if (!underlyingType->Clone(GetElementPointer(outObject, i), otherType->GetElementPointer(otherObject.GetData(), i)))
        {
            return false;
        }
    }

    return true;
}

bool IResizableArrayType::Serialize(const void* object, IConfig& config, ConfigValue& outValue, SerializationContext& context) const
{
    using namespace Common;

    const Type* elementType = GetUnderlyingType();
    const uint32 arraySize = GetArraySize(object);

    NFE_ASSERT(elementType, "Invalid DynArray element type");
    NFE_ASSERT(object, "Trying to serialize nullptr");

    // serialize array elements
    ConfigArray configArray;
    for (uint32 i = 0; i < arraySize; ++i)
    {
        ConfigValue arrayElementValue;
        const void* elementPtr = GetElementPointer(object, i);
        if (!elementType->Serialize(elementPtr, config, arrayElementValue, context))
        {
            NFE_LOG_ERROR("Failed to serialize DynArray element (index %u/%u)", i, arraySize);
            return false;
        }
        config.AddValue(configArray, arrayElementValue);
    }

    // success
    outValue = configArray;
    return true;
}

bool IResizableArrayType::Deserialize(void* outObject, const IConfig& config, const ConfigValue& value, SerializationContext& context) const
{
    using namespace Common;

    const Type* elementType = GetUnderlyingType();

    NFE_ASSERT(elementType, "Invalid native array element type");
    NFE_ASSERT(outObject, "Trying to deserialize to nullptr");

    if (!value.IsArray())
    {
        NFE_LOG_ERROR("Expected array type");
        return false;
    }

    const ConfigGenericValue arrayValue(&config, value);

    if (!ResizeArray(outObject, arrayValue.GetSize()))
    {
        return false;
    }

    uint32 numDeserializedArrayElements = 0;
    auto arrayIteratorCallback = [&] (uint32 index, const ConfigValue& arrayElement)
    {
        void* elementPtr = GetElementPointer(outObject, index);
        if (!elementType->Deserialize(elementPtr, config, arrayElement, context))
        {
            NFE_LOG_ERROR("Failed to parse native array element at index %i", index);
            return false;
        }

        numDeserializedArrayElements++;
        return true;
    };

    config.IterateArray(arrayIteratorCallback, value.GetArray());
    return true;
}

bool IResizableArrayType::SerializeBinary(const void* object, OutputStream* stream, SerializationContext& context) const
{
    const uint32 arraySize = GetArraySize(object);

    if (!context.IsMapping())
    {
        if (!stream->WriteCompressedUint(arraySize))
        {
            return false;
        }
    }

    if (arraySize > 0u)
    {
        const Type* underlyingType = GetUnderlyingType();
        const size_t elementSize = underlyingType->GetSize();
        const uint8* firstElementData = reinterpret_cast<const uint8*>(GetElementPointer(object, 0u));

        if (underlyingType->CanBeMemcopied())
        {
            if (!context.IsMapping())
            {
                // bulk serialize whole array if can be memcopied
                size_t numBytesToWrite = elementSize * arraySize;
                if (numBytesToWrite != stream->Write(firstElementData, numBytesToWrite))
                {
                    return false;
                }
            }
        }
        else
        {
            for (uint32 i = 0; i < arraySize; ++i)
            {
                if (!underlyingType->SerializeBinary(firstElementData + elementSize * i, stream, context))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool IResizableArrayType::DeserializeBinary(void* outObject, InputStream& stream, SerializationContext& context) const
{
    uint32 arraySize;
    if (!stream.ReadCompressedUint(arraySize))
    {
        return false;
    }

    // TODO this requires elements to be copyable: would be nice to be able push back them using move constructor
    if (!ResizeArray(outObject, arraySize))
    {
        return false;
    }

    if (arraySize > 0u)
    {
        const Type* underlyingType = GetUnderlyingType();
        const size_t elementSize = underlyingType->GetSize();
        uint8* firstElementData = reinterpret_cast<uint8*>(GetElementPointer(outObject, 0u));

        if (underlyingType->CanBeMemcopied())
        {
            // bulk deserialize whole array if can be memcopied
            size_t numBytesToRead = elementSize * arraySize;
            if (numBytesToRead != stream.Read(firstElementData, numBytesToRead))
            {
                return false;
            }
        }
        else
        {
            for (uint32 i = 0; i < arraySize; ++i)
            {
                if (!underlyingType->DeserializeBinary(firstElementData + elementSize * i, stream, context))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool IResizableArrayType::SerializeTypeName(Common::OutputStream* stream, SerializationContext& context) const
{
    // write header
    if (!Type::SerializeTypeName(stream, context))
    {
        return false;
    }

    // append inner type
    return mUnderlyingType->SerializeTypeName(stream, context);
}

bool IResizableArrayType::Compare(const void* objectA, const void* objectB) const
{
    const uint32 arraySizeA = GetArraySize(objectA);
    const uint32 arraySizeB = GetArraySize(objectB);
    if (arraySizeA != arraySizeB)
    {
        return false;
    }

    const Type* underlyingType = GetUnderlyingType();
    for (uint32 i = 0; i < arraySizeA; ++i)
    {
        if (!underlyingType->Compare(GetElementPointer(objectA, i), GetElementPointer(objectB, i)))
        {
            return false;
        }
    }

    return true;
}

bool IResizableArrayType::Clone(void* destObject, const void* sourceObject) const
{
    const Type* underlyingType = GetUnderlyingType();
    const uint32 targetSize = GetArraySize(sourceObject);

    // resize destination array object to reserve space
    if (!ResizeArray(destObject, targetSize))
    {
        return false;
    }

    // clone array elements
    bool success = true;

    if (targetSize > 0u)
    {
        if (underlyingType->CanBeMemcopied())
        {
            const size_t bytesToCopy = targetSize * underlyingType->GetSize();
            memcpy(GetElementPointer(destObject, 0), GetElementPointer(sourceObject, 0), bytesToCopy);
        }
        else
        {
            // clone array elements
            for (uint32 i = 0; i < targetSize; ++i)
            {
                if (!underlyingType->Clone(GetElementPointer(destObject, i), GetElementPointer(sourceObject, i)))
                {
                    success = false;
                }
            }
        }
    }

    return success;
}

} // namespace RTTI
} // namespace NFE
