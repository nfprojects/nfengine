/**
 * @file
 * @author Witek902
 * @brief  Definition of ResizableArrayType.
 */

#pragma once

#include "ReflectionArrayType.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../../Containers/DynArray.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type information for array types that can be resized (DynArray, StaticArray).
 */
class NFCOMMON_API ResizableArrayType : public IArrayType
{
    NFE_MAKE_NONCOPYABLE(ResizableArrayType)

public:
    NFE_FORCE_INLINE ResizableArrayType(const Type* underlyingType)
        : IArrayType(underlyingType)
    { }

    // get number of array elements
    virtual uint32 GetArraySize(const void* arrayObject) const;

    // get maximum capacity (theoretical, may exceede memory limits)
    virtual uint32 GetMaxCapacity() const;

    // resize the array
    virtual bool ResizeArray(void* arrayObject, uint32 targetSize) const;

    // Type interface implementation
    virtual bool Serialize(const void* object, Common::IConfig& config, Common::ConfigValue& outValue, SerializationContext& context) const override final;
    virtual bool Deserialize(void* outObject, const Common::IConfig& config, const Common::ConfigValue& value, SerializationContext& context) const override final;
    virtual bool SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const override final;
    virtual bool DeserializeBinary(void* outObject, Common::InputStream& stream, SerializationContext& context) const override final;
    virtual bool SerializeTypeName(Common::OutputStream* stream, SerializationContext& context) const override final;
    virtual bool TryLoadFromDifferentType(void* outObject, const VariantView& otherObject) const override final;
    virtual bool Compare(const void* objectA, const void* objectB) const override final;
    virtual bool Clone(void* destObject, const void* sourceObject) const override final;
};

} // namespace RTTI
} // namespace NFE
