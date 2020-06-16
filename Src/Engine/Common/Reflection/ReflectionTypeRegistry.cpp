/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of reflection system's type registry.
 */

#include "PCH.hpp"
#include "ReflectionTypeRegistry.hpp"


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
    const auto iter = mTypesByHash.Find(hash);

    if (iter == mTypesByHash.End())
    {
        return nullptr;
    }

    return iter->second;
}

const Type* TypeRegistry::GetExistingType(const char* name) const
{
    return GetExistingType(StringView(name));
}

struct LastAccessedType
{
    StringView name;
    const Type* type = nullptr;
};

// TODO cache more than one?
static thread_local LastAccessedType s_lastAccessedCache;

const Type* TypeRegistry::GetExistingType(const StringView name) const
{
    // check if in cache (to avoid accessing this huge hash map below)
    if (s_lastAccessedCache.name == name)
    {
        return s_lastAccessedCache.type;
    }

    const auto iter = mTypesByName.Find(name);

    if (iter == mTypesByName.End())
    {
        return nullptr;
    }

    s_lastAccessedCache.name = name;
    s_lastAccessedCache.type = iter->second;

    return iter->second;
}

const Type* TypeRegistry::RegisterType(size_t hash, Type* type)
{
    NFE_ASSERT(type, "Invalid type pointer");

    const auto iter = mTypesByHash.Find(hash);

    if (iter != mTypesByHash.End())
    {
        const StringView name = iter->second->GetName();
        NFE_FATAL("Type with given hash already exists (%.*s)", name.Length(), name.Data());
    }

    mTypesByHash.Insert(hash, type);
    mTypesByName.Insert(StringView(type->GetName()), type);

    return type;
}

void TypeRegistry::Cleanup()
{
    NFE_LOG_INFO("RTTI: Cleaning registered types list");

    mTypesByName.Clear();
    mTypesByHash.Clear();
}

bool TypeRegistry::SerializeTypeName(const Type* type, Common::OutputStream* stream, SerializationContext& context) const
{
    NFE_UNUSED(type);
    NFE_UNUSED(stream);
    NFE_UNUSED(context);

    return false;
}

bool TypeRegistry::DeserializeTypeName(const Type** outType, Common::InputStream* stream, SerializationContext& context)
{
    NFE_UNUSED(outType);
    NFE_UNUSED(stream);
    NFE_UNUSED(context);

    return false;
}

} // namespace RTTI
} // namespace NFE
