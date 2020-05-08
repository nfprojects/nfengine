/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionFundamentalType.hpp"
#include "../SerializationContext.hpp"

namespace NFE {
namespace RTTI {

bool FundamentalType::SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const
{
    if (context.IsMapping())
    {
        // don't serialize anything in mapping stage
        return true;
    }

    return GetSize() == stream->Write(object, GetSize());
}

bool FundamentalType::DeserializeBinary(void* outObject, Common::InputStream& stream, const SerializationContext& context) const
{
    NFE_UNUSED(context);

    return GetSize() == stream.Read(outObject, GetSize());
}

} // namespace RTTI
} // namespace NFE
