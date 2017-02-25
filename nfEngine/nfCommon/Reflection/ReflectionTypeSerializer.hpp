/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's ITypeSerializer interface.
 */

#pragma once

#include "../ForwardDeclarations.hpp"


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
     * @param   object  Source object to serialize. Will be always valid.
     * @param   stream  Target binary stream.
     */
    virtual bool Serialize(const void* object, Common::OutputStream& stream) const = 0;

    /**
     * Deserialize object from binary stream.
     * @param   outObject   Target object to deserialize. Will be always valid.
     * @param   stream      Source binary stream.
     */
    virtual bool Deserialize(void* outObject, Common::InputStream& stream) const = 0;

    /**
     * Serialize object to config structure.
     * @param   object          Source object to serialize. Will be always valid.
     * @param   config,outValue Target config value.
     */
    virtual bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const = 0;

    /**
     * Deserialize object from config structure.
     * @param   outObject       Target object to deserialize. Will be always valid.
     * @param   config,value    Source config value.
     */
    virtual bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const = 0;
};

} // namespace RTTI
} // namespace NFE
