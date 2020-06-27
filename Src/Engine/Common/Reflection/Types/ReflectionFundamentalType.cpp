/**
 * @file
 */

#include "PCH.hpp"
#include "ReflectionFundamentalType.hpp"
#include "../SerializationContext.hpp"
#include "../ReflectionTypeResolver.hpp"
#include "../ReflectionVariant.hpp"
#include "../../Utils/Stream/InputStream.hpp"
#include "../../Utils/Stream/OutputStream.hpp"

namespace NFE {
namespace RTTI {

FundamentalType::FundamentalType() = default;

bool FundamentalType::SerializeBinary(const void* object, Common::OutputStream* stream, SerializationContext& context) const
{
    if (context.IsMapping())
    {
        // don't serialize anything in mapping stage
        return true;
    }

    return GetSize() == stream->Write(object, GetSize());
}

bool FundamentalType::DeserializeBinary(void* outObject, Common::InputStream& stream, SerializationContext& context) const
{
    NFE_UNUSED(context);

    return GetSize() == stream.Read(outObject, GetSize());
}

template<typename DestType, typename SrcType>
static bool TryUpgradeFundamentalType(void* outObject, const FundamentalType* outType, const Variant& otherObject)
{
    if (GetType<DestType>() == outType && GetType<SrcType>() == otherObject.GetType())
    {
        *reinterpret_cast<DestType*>(outObject) = otherObject.Get<SrcType>();
        return true;
    }
    return false;
}

bool FundamentalType::TryLoadFromDifferentType(void* outObject, const Variant& otherObject) const
{
    // upgrading unsigned int to a bigger type is safe
    if (TryUpgradeFundamentalType<uint64, uint32>(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<uint64, uint16>(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<uint64, uint8 >(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<uint32, uint16>(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<uint32, uint8 >(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<uint16, uint8 >(outObject, this, otherObject)) return true;

    // upgrading signed int to a bigger type is safe
    if (TryUpgradeFundamentalType<int64, int32>(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<int64, int16>(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<int64, int8 >(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<int32, int16>(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<int32, int8 >(outObject, this, otherObject)) return true;
    if (TryUpgradeFundamentalType<int16, int8 >(outObject, this, otherObject)) return true;

    // upgrading float to double
    if (TryUpgradeFundamentalType<double, float>(outObject, this, otherObject)) return true;

    // assume other conversions are not safe
    return false;
}

} // namespace RTTI
} // namespace NFE
