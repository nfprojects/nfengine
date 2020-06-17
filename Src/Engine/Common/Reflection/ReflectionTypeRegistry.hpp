/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's types registry.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ReflectionTypeResolver.hpp"
#include "Types/ReflectionType.hpp"

#include "../Containers/HashMap.hpp"
#include "../Containers/StringView.hpp"


namespace NFE {
namespace RTTI {

enum class TypeDeserializationResult : uint8
{
    Success = 0,    // Note: deserialize could be still "nullptr", which means it was serialized like that
    UnknownType,    // There was some valid serialized but now it's not known (missing type at runtime)
    Error,          // Deserialization failed due to corrupted data or bug in code. Any further operations on the stream should be aborted
};

/**
 * Container for all the registered types.
 */
class NFCOMMON_API TypeRegistry : public ITypeRegistry
{
public:
    /**
     * Get the registry singleton instance.
     */
    static TypeRegistry& GetInstance();

    /**
     * Find existing type by typeid hash.
     */
    virtual const Type* GetExistingType(size_t hash) const override;

    /**
     * Find existing type by name.
     */
    virtual const Type* GetExistingType(const char* name) const override;
    const Type* GetExistingType(const Common::StringView name) const;

    /**
     * Register non-existing type.
     */
    virtual const Type* RegisterType(size_t hash, Type* type) override;

    /**
     * Unregister all the types.
     */
    void Cleanup();

    // write type name in binary form
    bool SerializeTypeName(const Type* type, Common::OutputStream* stream, SerializationContext& context) const;

    // read type name from binary form
    TypeDeserializationResult DeserializeTypeName(const Type*& outType, Common::InputStream& stream, const SerializationContext& context);

private:
    TypeRegistry() = default;

    Common::HashMap<Common::StringView, const Type*> mTypesByName;
    Common::HashMap<size_t, Type*> mTypesByHash;
};


} // namespace RTTI
} // namespace NFE
