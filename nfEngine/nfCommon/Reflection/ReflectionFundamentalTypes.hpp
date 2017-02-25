/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of fundamental types and their serialization.
 */

#pragma once

#include "ReflectionMacros.hpp"
#include "ReflectionTypeSerializer.hpp"
#include "../Config/ConfigValue.hpp"
#include "../Utils/InputStream.hpp"
#include "../Utils/OutputStream.hpp"


namespace NFE {
namespace RTTI {

/**
 * Type serializer for fundamental types.
 */
template<typename T>
class FundamentalTypeSerializer final : public ITypeSerializer
{
public:
    bool Serialize(const void* object, Common::OutputStream& stream) const override
    {
        // TODO [memory optimization] consider using some variable length encoding for integers

        const size_t dataSize = sizeof(T);
        return stream.Write(&object, dataSize) == dataSize;
    }

    bool Deserialize(void* outObject, Common::InputStream& stream) const override
    {
        const size_t dataSize = sizeof(T);

        // read to local object so the target is updated atomically
        T localObject;
        if (stream.Read(&localObject, dataSize) == dataSize)
        {
            T* typedObject = static_cast<T*>(outObject);
            *typedObject = localObject;
            return true;
        }

        return false;
    }

    bool Serialize(const void* object, Common::Config& config, Common::ConfigValue& outValue) const override
    {
        // no need to access Config object itself, all the data for fundamental types is contained in ConfigValue
        UNUSED(config);

        const T* typedObject = static_cast<const T*>(object);
        outValue = Common::ConfigValue(*typedObject);
        return true;
    }

    bool Deserialize(void* outObject, const Common::Config& config, const Common::ConfigValue& value) const override
    {
        UNUSED(config);

        // TODO type casting (with check if there is no overflow)
        if (value.Is<T>())
        {
            T* typedObject = static_cast<T*>(outObject);
            *typedObject = value.Get<T>();
            return true;
        }

        return false;
    }
};

} // namespace RTTI
} // namespace NFE


//////////////////////////////////////////////////////////////////////////


NFE_DECLARE_TYPE(bool)
NFE_DECLARE_TYPE(float)
NFE_DECLARE_TYPE(double)

NFE_DECLARE_TYPE(NFE::uint8)
NFE_DECLARE_TYPE(NFE::uint16)
NFE_DECLARE_TYPE(NFE::uint32)
NFE_DECLARE_TYPE(NFE::uint64)

NFE_DECLARE_TYPE(NFE::int8)
NFE_DECLARE_TYPE(NFE::int16)
NFE_DECLARE_TYPE(NFE::int32)
NFE_DECLARE_TYPE(NFE::int64)
