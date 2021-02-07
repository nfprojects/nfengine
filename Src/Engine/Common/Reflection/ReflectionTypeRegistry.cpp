/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's type registry.
 */

#include "PCH.hpp"
#include "ReflectionTypeRegistry.hpp"
#include "ReflectionTypeResolver.hpp"
#include "SerializationContext.hpp"
#include "Types/ReflectionFundamentalType.hpp"
#include "Types/ReflectionDynArrayType.hpp"
#include "Types/ReflectionStaticArrayType.hpp"
#include "Types/ReflectionNativeArrayType.hpp"
#include "Types/ReflectionSharedPtrType.hpp"
#include "Types/ReflectionUniquePtrType.hpp"
#include "Types/ReflectionStringType.hpp"
#include "../Utils/Stream/OutputStream.hpp"
#include "../Utils/Stream/InputStream.hpp"
#include "../Utils/ScopedLock.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

ITypeRegistry& ITypeRegistry::GetInstance()
{
    return TypeRegistry::GetInstance();
}

TypeRegistry& TypeRegistry::GetInstance()
{
    static TypeRegistry registry;
    return registry;
}

const Type* TypeRegistry::GetExistingType(size_t hash) const
{
    NFE_SCOPED_SHARED_LOCK(mTypesListLock);
    return GetExistingType_NoLock(hash);
}

const Type* TypeRegistry::GetExistingType_NoLock(size_t hash) const
{
    const auto iter = mTypesByHash.Find(hash);

    if (iter == mTypesByHash.End())
    {
        return nullptr;
    }

    return iter->second;
}

const Type* TypeRegistry::GetExistingType(const char* name) const
{
    NFE_SCOPED_SHARED_LOCK(mTypesListLock);
    return GetExistingType_NoLock(name);
}

const Type* TypeRegistry::GetExistingType_NoLock(const char* name) const
{
    return GetExistingType_NoLock(StringView(name));
}

struct LastAccessedType
{
    String name;
    const Type* type = nullptr;
};

const Type* TypeRegistry::GetExistingType_NoLock(const StringView name) const
{
    // TODO cache more than one?
    static thread_local LastAccessedType s_lastAccessedCache;

    const Type* foundType = nullptr;

    // check if in cache (to avoid accessing this huge hash map below)
    if (s_lastAccessedCache.name == name)
    {
        foundType = s_lastAccessedCache.type;
    }
    else
    {
        const auto iter = mTypesByName.Find(name);

        if (iter == mTypesByName.End())
        {
            return nullptr;
        }

        foundType = iter->second;

        s_lastAccessedCache.name = name;
        s_lastAccessedCache.type = foundType;
    }

    if (!foundType->IsEnabled())
    {
        foundType = nullptr;
    }

    return foundType;
}

void TypeRegistry::RegisterType(size_t hash, Type* type)
{
    NFE_SCOPED_LOCK(mTypesListLock);
    RegisterType_NoLock(hash, type);
}

void TypeRegistry::RegisterType_NoLock(size_t hash, Type* type)
{
    NFE_ASSERT(type, "Invalid type pointer");

    const auto iter = mTypesByHash.Find(hash);

    if (iter != mTypesByHash.End())
    {
        const StringView name = iter->second->GetName();
        NFE_FATAL("Type with given hash already exists (%.*s)", name.Length(), name.Data());
    }

    mTypesByHash.Insert(hash, type);
}

void TypeRegistry::RegisterTypeName(const StringView name, Type* type)
{
    NFE_SCOPED_LOCK(mTypesListLock);
    RegisterTypeName_NoLock(name, type);
}

void TypeRegistry::RegisterTypeName_NoLock(const StringView name, Type* type)
{
    const auto iter = mTypesByName.Find(name);

    if (iter != mTypesByName.End())
    {
        NFE_FATAL("Type with given name already exists (%.*s)", name.Length(), name.Data());
    }

    mTypesByName.Insert(name, type);
}

void TypeRegistry::Cleanup()
{
    NFE_LOG_INFO("RTTI: Cleaning registered types list");

    mTypesByName.Clear();
    mTypesByHash.Clear();
}

bool TypeRegistry::SerializeTypeName(const Type* type, OutputStream* stream, SerializationContext& context) const
{
    if (type)
    {
        return type->SerializeTypeName(stream, context);
    }
    else // empty type (e.g. shared ptr keeping nullptr)
    {
        if (stream)
        {
            TypeNameID typeNameID = TypeNameID::Invalid;
            return stream->Write(reinterpret_cast<uint8&>(typeNameID));
        }
    }
    return true;
}

// inverse of ClassType::SerializeTypeName
TypeDeserializationResult TypeRegistry::DeserializeClassType(const Type*& outType, InputStream& stream, SerializationContext& context)
{
    uint32 strIndex;
    if (!stream.ReadCompressedUint(strIndex))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return TypeDeserializationResult::Error;
    }
    StringView serializedTypeName;
    if (!context.UnmapString(strIndex, serializedTypeName))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return TypeDeserializationResult::Error;
    }
    
    {
        NFE_SCOPED_SHARED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(serializedTypeName);
    }

    if (outType)
    {
        return outType->GetKind() == TypeKind::Class ? TypeDeserializationResult::Success : TypeDeserializationResult::Error;
    }

    return TypeDeserializationResult::UnknownType;
}

// inverse of EnumType::SerializeTypeName
TypeDeserializationResult TypeRegistry::DeserializeEnumType(const Type*& outType, InputStream& stream, SerializationContext& context)
{
    uint32 strIndex;
    if (!stream.ReadCompressedUint(strIndex))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return TypeDeserializationResult::Error;
    }
    StringView serializedTypeName;
    if (!context.UnmapString(strIndex, serializedTypeName))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return TypeDeserializationResult::Error;
    }

    {
        NFE_SCOPED_SHARED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(serializedTypeName);
    }

    if (outType)
    {
        return outType->GetKind() == TypeKind::Enumeration ? TypeDeserializationResult::Success : TypeDeserializationResult::Error;
    }

    return TypeDeserializationResult::UnknownType;
}

TypeDeserializationResult TypeRegistry::DeserializeArrayType(const Type*& outType, InputStream& stream, SerializationContext& context)
{
    const Type* underlyingType = nullptr;

    TypeDeserializationResult result = DeserializeTypeName(underlyingType, stream, context);
    if (result != TypeDeserializationResult::Success)
    {
        return result;
    }

    const String expectedTypeName = DynArrayType::BuildTypeName(underlyingType);

    {
        NFE_SCOPED_SHARED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(expectedTypeName);
    }

    if (!outType)
    {
        NFE_SCOPED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(expectedTypeName);

        if (!outType)
        {
            Type* newType = new DynArrayType(underlyingType);

            TypeInfo typeInfo;
            typeInfo.kind = TypeKind::Array;
            typeInfo.typeNameID = TypeNameID::Array;
            typeInfo.dynamicType = true;
            typeInfo.size = sizeof(DynArray<int32>);
            typeInfo.alignment = alignof(DynArray<int32>);
            typeInfo.name = expectedTypeName.Str();

            newType->Initialize(typeInfo);

            RegisterTypeName_NoLock(newType->GetName(), newType);

            outType = newType;
        }
    }

    return TypeDeserializationResult::Success;
}

TypeDeserializationResult TypeRegistry::DeserializeNativeArrayType(const Type*& outType, InputStream& stream, SerializationContext& context)
{
    const Type* underlyingType = nullptr;

    uint32 arraySize = 0;
    if (!stream.ReadCompressedPositiveInt(arraySize))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return TypeDeserializationResult::Error;
    }

    TypeDeserializationResult result = DeserializeTypeName(underlyingType, stream, context);
    if (result != TypeDeserializationResult::Success)
    {
        return result;
    }

    const String expectedTypeName = NativeArrayType::BuildTypeName(underlyingType, arraySize);

    {
        NFE_SCOPED_SHARED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(expectedTypeName);
    }

    if (!outType)
    {
        NFE_SCOPED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(expectedTypeName);

        if (!outType)
        {
            Type* newType = new NativeArrayType(arraySize, underlyingType);

            TypeInfo typeInfo;
            typeInfo.kind = TypeKind::Array;
            typeInfo.typeNameID = TypeNameID::NativeArray;
            typeInfo.size = arraySize * underlyingType->GetSize();
            typeInfo.alignment = underlyingType->GetAlignment();
            typeInfo.name = expectedTypeName.Str();

            newType->Initialize(typeInfo);

            RegisterTypeName_NoLock(newType->GetName(), newType);

            outType = newType;
        }
    }

    return TypeDeserializationResult::Success;
}

TypeDeserializationResult TypeRegistry::DeserializeUniquePtrType(const Type*& outType, InputStream& stream, SerializationContext& context)
{
    const Type* underlyingType = nullptr;

    TypeDeserializationResult result = DeserializeTypeName(underlyingType, stream, context);
    if (result != TypeDeserializationResult::Success)
    {
        return result;
    }

    const String expectedTypeName = UniquePtrType::BuildTypeName(underlyingType);

    {
        NFE_SCOPED_SHARED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(expectedTypeName);
    }

    if (!outType)
    {
        NFE_SCOPED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(expectedTypeName);

        if (!outType)
        {
            Type* newType = new UniquePtrType(underlyingType);

            TypeInfo typeInfo;
            typeInfo.kind = TypeKind::UniquePtr;
            typeInfo.typeNameID = TypeNameID::UniquePtr;
            typeInfo.size = sizeof(UniquePtr<int32>);
            typeInfo.alignment = alignof(UniquePtr<int32>);
            typeInfo.name = expectedTypeName.Str();

            newType->Initialize(typeInfo);

            RegisterTypeName_NoLock(newType->GetName(), newType);

            outType = newType;
        }
    }

    return TypeDeserializationResult::Success;
}

TypeDeserializationResult TypeRegistry::DeserializeSharedPtrType(const Type*& outType, InputStream& stream, SerializationContext& context)
{
    const Type* underlyingType = nullptr;

    TypeDeserializationResult result = DeserializeTypeName(underlyingType, stream, context);
    if (result != TypeDeserializationResult::Success)
    {
        return result;
    }

    const String expectedTypeName = SharedPtrType::BuildTypeName(underlyingType);

    {
        NFE_SCOPED_SHARED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(expectedTypeName);
    }

    {
        NFE_SCOPED_LOCK(mTypesListLock);
        outType = GetExistingType_NoLock(expectedTypeName);

        if (!outType)
        {
            Type* newType = new UniquePtrType(underlyingType);

            TypeInfo typeInfo;
            typeInfo.kind = TypeKind::SharedPtr;
            typeInfo.typeNameID = TypeNameID::SharedPtr;
            typeInfo.size = sizeof(SharedPtr<int32>);
            typeInfo.alignment = alignof(SharedPtr<int32>);
            typeInfo.name = expectedTypeName.Str();

            newType->Initialize(typeInfo);

            RegisterTypeName_NoLock(newType->GetName(), newType);

            outType = newType;
        }
    }

    return TypeDeserializationResult::Success;
}

TypeDeserializationResult TypeRegistry::DeserializeTypeName(const Type*& outType, InputStream& stream, SerializationContext& context)
{
    // read header
    TypeNameID typeNameID = TypeNameID::Invalid;
    if (!stream.Read(reinterpret_cast<uint8&>(typeNameID)))
    {
        NFE_LOG_ERROR("Deserialization failed. Corrupted data?");
        return TypeDeserializationResult::Error;
    }

    TypeDeserializationResult result = TypeDeserializationResult::Success;

    // this may look ugly, but this way of serialization greately reduces size of binary stream
    switch (typeNameID)
    {
    case TypeNameID::Invalid:
        outType = nullptr;
        break;
    case TypeNameID::Fundamental_Bool:
        outType = GetType<bool>();
        break;
    case TypeNameID::Fundamental_Uint8:
        outType = GetType<uint8>();
        break;
    case TypeNameID::Fundamental_Uint16:
        outType = GetType<uint16>();
        break;
    case TypeNameID::Fundamental_Uint32:
        outType = GetType<uint32>();
        break;
    case TypeNameID::Fundamental_Uint64:
        outType = GetType<uint64>();
        break;
    case TypeNameID::Fundamental_Int8:
        outType = GetType<int8>();
        break;
    case TypeNameID::Fundamental_Int16:
        outType = GetType<int16>();
        break;
    case TypeNameID::Fundamental_Int32:
        outType = GetType<int32>();
        break;
    case TypeNameID::Fundamental_Int64:
        outType = GetType<int64>();
        break;
    case TypeNameID::Fundamental_Float:
        outType = GetType<float>();
        break;
    case TypeNameID::Fundamental_Double:
        outType = GetType<double>();
        break;
    case TypeNameID::String:
        outType = GetType<String>();
        break;
    case TypeNameID::Class:
        result = DeserializeClassType(outType, stream, context);
        break;
    case TypeNameID::Enumeration:
        result = DeserializeEnumType(outType, stream, context);
        break;
    case TypeNameID::NativeArray:
        result = DeserializeNativeArrayType(outType, stream, context);
        break;
    case TypeNameID::Array:
        result = DeserializeArrayType(outType, stream, context);
        break;
    case TypeNameID::UniquePtr:
        result = DeserializeUniquePtrType(outType, stream, context);
        break;
    case TypeNameID::SharedPtr:
        result = DeserializeSharedPtrType(outType, stream, context);
        break;
    default:
        outType = nullptr;
        result = TypeDeserializationResult::Error;
        NFE_LOG_ERROR("Deserialization failed. Invalid type name ID: %u. Corrupted data?", (uint32)typeNameID);
    }

    if (result == TypeDeserializationResult::Success)
    {
        // sanity check
        NFE_ASSERT(!outType || outType->GetTypeNameID() == typeNameID, "Invalid type name ID");
    }

    return result;
}

} // namespace RTTI
} // namespace NFE
