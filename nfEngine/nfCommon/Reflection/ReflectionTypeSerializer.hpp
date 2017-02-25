/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's ITypeSerializer interface.
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace RTTI {

/**
 * An interface used for build-in types serialization.
 */
class ITypeSerializer
{
public:
    virtual ~ITypeSerializer() { }

    /**
     * Serialize object to binary stream.
     */
    virtual bool Serialize(const void* object, Common::OutputStream& stream) const = 0;

    /**
     * Deserialize object from binary stream.
     */
    virtual bool Deserialize(void* outObject, Common::InputStream& stream) const = 0;

    /**
     * Serialize object to config structure.
     */
    virtual bool Serialize(const void* object, Common::ConfigValue& outValue) const = 0;

    /**
     * Deserialize object from config structure.
     */
    virtual bool Deserialize(void* outObject, const Common::ConfigValue& value) const = 0;
};


} // namespace RTTI
} // namespace NFE
