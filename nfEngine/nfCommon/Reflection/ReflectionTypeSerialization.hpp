/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Type class.
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Config/Config.hpp"

namespace NFE {
namespace RTTI {


/**
 * An interface providing fundamental types serialization capability.
 */
template<typename T>
class TypeSerialization : public ITypeSerialization
{
public:
    // serialize object to binary stream
    bool Serialize(const char* name, const void* object, Common::OutputStream& stream) const override
    {
        // TODO
        UNUSED(name);
        UNUSED(object);
        UNUSED(stream);
        return false;
    }

    // deserialize object from binary stream
    bool Deserialize(void* outObject, Common::InputStream& stream) const override
    {
        // TODO
        UNUSED(outObject);
        UNUSED(stream);
        return false;
    }

    // serialize object to config structure
    bool Serialize(const char* name, const void* object, Common::Config& config, Common::ConfigObject& configObject) const override
    {
        const T* typedObject = static_cast<const T*>(object);
        config.AddValue(configObject, name, Common::ConfigValue(*typedObject));
        return true;
    }

    // deserialize object from config structure
    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigObject& configObject) const override
    {
        // TODO
        UNUSED(outObject);
        UNUSED(config);
        UNUSED(configObject);
        return false;
    }
};

} // namespace RTTI
} // namespace NFE
