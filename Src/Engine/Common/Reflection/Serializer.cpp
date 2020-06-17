#include "PCH.hpp"
#include "Serializer.hpp"
#include "SerializationContext.hpp"
#include "ReflectionTypeRegistry.hpp"
#include "Types/ReflectionClassType.hpp"
#include "../Utils/Stream/OutputStream.hpp"
#include "../Utils/Stream/InputStream.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

static const uint32 c_magicValue = 'KPfn'; // nfPK

bool Serialize(const ObjectPtr& rootObject, OutputStream& outputStream)
{
    const ArrayView<const ObjectPtr> rootObjects(&rootObject, 1);
    return Serialize(rootObjects, outputStream);
}

bool Serialize(const ArrayView<const ObjectPtr> rootObjects, OutputStream& outputStream)
{
    NFE_ASSERT(!rootObjects.Empty(), "What's point in serializing zero objects?");

    TypeRegistry& typeRegistry = TypeRegistry::GetInstance();

    // write magic
    if (!outputStream.Write(c_magicValue))
    {
        return false;
    }

    // write current version
    if (!outputStream.WriteCompressedUint(SerializationContext::CurrentVersion))
    {
        return false;
    }

    SerializationContext serializationContext;
    DynArray<uint32> rootObjectMappedIndices;

    // Mapping: objects and strings are collected into serialization context
    {
        serializationContext.InitStage(SerializationContext::Stage::Mapping);

        for (const ObjectPtr& rootObjectPtr : rootObjects)
        {
            const Type* objectClassType = rootObjectPtr->GetDynamicType();

            // map everything recursively
            // note: not passing stream, because we don't serialize yet
            if (!objectClassType->SerializeBinary(rootObjectPtr.Get(), nullptr, serializationContext))
            {
                NFE_LOG_ERROR("Serialize: Mapping stage failed");
                return false;
            }

            // map root object and it's type name
            typeRegistry.SerializeTypeName(objectClassType, nullptr, serializationContext);
            const uint32 objectIndex = serializationContext.MapObject(rootObjectPtr);

            rootObjectMappedIndices.PushBack(objectIndex);
        }
    }

    // TODO detect circular object dependencies

    // serialize strings table
    {
        serializationContext.OptimizeStringTable();

        const uint8 nullTerminator = 0u;
        const auto& stringTable = serializationContext.GetMappedStrings();

        uint32 stringTableSizeInBytes = 0;
        for (const StringView& str : stringTable)
        {
            stringTableSizeInBytes += str.Length();
            stringTableSizeInBytes += 1; // null termination
        }

        if (!outputStream.WriteCompressedUint(stringTableSizeInBytes))
        {
            return false;
        }

        for (const StringView& str : stringTable)
        {
            if (str.Length() != outputStream.Write(str.Data(), str.Length()))
            {
                return false;
            }

            if (!outputStream.Write(nullTerminator))
            {
                return false;
            }
        }
    }

    // Serialization: objects data are actually serialized to binary stream here
    {
        serializationContext.OptimizeObjectTable();

        serializationContext.InitStage(SerializationContext::Stage::Serialization);

        const auto& objectTable = serializationContext.GetMappedObjects();

        if (!outputStream.WriteCompressedUint(objectTable.Size()))
        {
            return false;
        }

        for (const ObjectPtr& objectPtr : objectTable)
        {
            const Type* objectClassType = objectPtr->GetDynamicType();

            if (!typeRegistry.SerializeTypeName(objectClassType, &outputStream, serializationContext))
            {
                return false;
            }

            // serialize everything recursively
            if (!objectClassType->SerializeBinary(objectPtr.Get(), &outputStream, serializationContext))
            {
                NFE_LOG_ERROR("Serialize: Serialization stage failed");
                return false;
            }
        }
    }

    // Write root object indices
    {
        if (!outputStream.WriteCompressedUint(rootObjectMappedIndices.Size()))
        {
            return false;
        }

        for (const uint32 rootObjectIndex : rootObjectMappedIndices)
        {
            if (!outputStream.WriteCompressedUint(rootObjectIndex))
            {
                return false;
            }
        }
    }

    // TODO write data buffers

    return true;
}

bool Deserialize(DynArray<ObjectPtr>& outRootObjects, InputStream& inputStream)
{
    TypeRegistry& typeRegistry = TypeRegistry::GetInstance();

    // deseiralize magic
    {
        uint32 magic;
        if (!inputStream.Read(magic))
        {
            NFE_LOG_ERROR("Deserialize: Failed to read magic");
            return false;
        }

        if (magic != c_magicValue)
        {
            NFE_LOG_ERROR("Deserialize: Invalid magic value");
            return false;
        }
    }

    uint32 streamVersion;
    if (!inputStream.ReadCompressedUint(streamVersion))
    {
        NFE_LOG_ERROR("Deserialize: Failed to read stream version");
        return false;
    }

    SerializationContext serializationContext(streamVersion);

    // deserialize strings table
    {
        uint32 stringTableSizeInBytes = 0;
        if (!inputStream.ReadCompressedUint(stringTableSizeInBytes))
        {
            NFE_LOG_ERROR("Deserialize: Failed to read string table size");
            return false;
        }

        Buffer stringBuffer;
        if (!stringBuffer.Resize(stringTableSizeInBytes))
        {
            NFE_LOG_ERROR("Deserialize: Failed to allocate string table");
            return false;
        }

        if (stringTableSizeInBytes != inputStream.Read(stringBuffer.Data(), stringTableSizeInBytes))
        {
            NFE_LOG_ERROR("Deserialize: Filed to read string table");
            return false;
        }

        if (!serializationContext.InitStringTable(std::move(stringBuffer)))
        {
            NFE_LOG_ERROR("Deserialize: Failed to parse string buffer");
            return false;
        }
    }

    // deserialize objects
    {
        serializationContext.InitStage(SerializationContext::Stage::Deserialization);

        uint32 objectTableSize = 0;
        if (!inputStream.ReadCompressedUint(objectTableSize))
        {
            NFE_LOG_ERROR("Deserialize: Failed to read object table size");
            return false;
        }

        for (uint32 objIndex = 0; objIndex < objectTableSize; ++objIndex)
        {
            const Type* serializedType = nullptr;
            TypeDeserializationResult typeDeserializationResult = typeRegistry.DeserializeTypeName(serializedType, inputStream, serializationContext);

            if (typeDeserializationResult == TypeDeserializationResult::Error)
            {
                NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
                return false;
            }

            // serialized type not known
            if (typeDeserializationResult == TypeDeserializationResult::UnknownType)
            {
                // TOOO skip object (will require storing skipping offsets on serializing)
                // TODO report unknown type
                NFE_LOG_ERROR("Deserialize: Failed to deserialize object %u: type in data is not known. Probably the type was removed from code.", objIndex);
                return false;
            }

            // handle nullptr
            if (nullptr == serializedType)
            {
                continue;
            }

            if (!serializedType->IsA(GetType<IObject>()))
            {
                // TOOO skip object (for now)
                // TODO report pointer type mismatch
                NFE_LOG_WARNING("Deserialize: Failed to deserialize object %u: Type in data '%s' is not inheriting from base type IObject",
                    objIndex, serializedType->GetName().Str());
                return false;
            }

            // create the object
            ObjectPtr outObject(serializedType->CreateObject<IObject>());

            // push into serialization context so it can be unmapped by dependent object
            serializationContext.PushObject(outObject);

            if (!outObject)
            {
                NFE_LOG_ERROR("Deserialize: Failed to allocate object %u", objIndex);
                return false;
            }

            // deserialize the object
            if (!serializedType->DeserializeBinary(outObject.Get(), inputStream, serializationContext))
            {
                NFE_LOG_ERROR("Deserialize: Failed to deserialize object %u", objIndex);
                return false;
            }
        }
    }

    // export root objects
    {
        uint32 numRootObjects = 0;
        if (!inputStream.ReadCompressedUint(numRootObjects))
        {
            NFE_LOG_ERROR("Deserialize: Failed to read number of root objects");
            return false;
        }

        outRootObjects.Clear();
        if (!outRootObjects.Resize(numRootObjects))
        {
            NFE_LOG_ERROR("Deserialize: Failed to allocate out objects array");
            return false;
        }

        for (uint32 i = 0; i < numRootObjects; ++i)
        {
            uint32 rootObjectIndex = 0;
            if (!inputStream.ReadCompressedUint(rootObjectIndex))
            {
                NFE_LOG_ERROR("Deserialize: Failed to read root object index for root object #%u", i);
                return false;
            }

            if (!serializationContext.UnmapObject(rootObjectIndex, outRootObjects[i]))
            {
                NFE_LOG_ERROR("Deserialize: Failed to unmap root object #%u (read index is %u). Corrupted data?", i, rootObjectIndex);
                return false;
            }
        }
    }

    return true;
}

} // namespace RTTI
} // namespace NFE
