#include "PCH.hpp"
#include "ReflectionMemberMetadataBuilder.hpp"
#include "ReflectionMember.hpp"
#include "ReflectionTypeRegistry.hpp"
#include "Types/ReflectionFundamentalType.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace RTTI {

using namespace Common;

static String MemberNameToHumanReadable(const char* name)
{
    const StringView nameView(name);
    
    String result;

    // skip initial "m" prefix
    bool skipPrefix = false;
    if (nameView.Length() > 1)
    {
        if (nameView[0] == 'm' && isupper(nameView[1]))
        {
            skipPrefix = true;
        }
    }

    if (skipPrefix)
    {
        result += nameView[1];
    }
    else
    {
        result += static_cast<char>(toupper(nameView[0]));
    }

    for (uint32 i = (skipPrefix ? 2 : 1); i < nameView.Length(); ++i)
    {
        if (isupper(nameView[i]) && islower(nameView[i - 1]))
        {
            result += ' ';
        }
        result += nameView[i];
    }

    return result;
}

// finalize metadata building
MemberMetadataBuilder::~MemberMetadataBuilder()
{
    if (mMember.mMetadata.descriptiveName.Empty())
    {
        mMember.mMetadata.descriptiveName = MemberNameToHumanReadable(mMember.GetName());
    }
}

MemberMetadataBuilder& MemberMetadataBuilder::Name(const char* name)
{
    NFE_ASSERT(mMember.mMetadata.descriptiveName.Empty(), "Descriptive name is already set");
    mMember.mMetadata.descriptiveName = name;

    return *this;
}

MemberMetadataBuilder& MemberMetadataBuilder::Min(double min)
{
    NFE_ASSERT(Math::IsValid(min), "Invalid min value");

    if (mMember.mType == GetType<uint8>())
    {
        NFE_ASSERT(min <= UINT8_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<uint16>())
    {
        NFE_ASSERT(min <= UINT16_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<uint32>())
    {
        NFE_ASSERT(min <= UINT32_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<uint64>())
    {
        NFE_ASSERT(min >= 0, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<int8>())
    {
        NFE_ASSERT(min >= INT8_MIN && min <= INT8_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<int16>())
    {
        NFE_ASSERT(min >= INT16_MIN && min <= INT16_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<int32>())
    {
        NFE_ASSERT(min >= INT32_MIN && min <= INT32_MAX, "Invalid value for give type");
    }

    NFE_ASSERT(0 == (mMember.mMetadata.flags & MemberMetadata_HasMinRange), "Property %s already has min range", mMember.GetName());
    mMember.mMetadata.flags = MemberMetadataFlags(mMember.mMetadata.flags | MemberMetadata_HasMinRange);

    NFE_ASSERT(min <= mMember.mMetadata.max, "Min must be less than max");
    mMember.mMetadata.min = min;

    return *this;
}

MemberMetadataBuilder& MemberMetadataBuilder::Max(double max)
{
    NFE_ASSERT(Math::IsValid(max), "Invalid max value");

    if (mMember.mType == GetType<uint8>())
    {
        NFE_ASSERT(max <= UINT8_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<uint16>())
    {
        NFE_ASSERT(max <= UINT16_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<uint32>())
    {
        NFE_ASSERT(max <= UINT32_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<uint64>())
    {
        NFE_ASSERT(max >= 0, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<int8>())
    {
        NFE_ASSERT(max >= INT8_MIN && max <= INT8_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<int16>())
    {
        NFE_ASSERT(max >= INT16_MIN && max <= INT16_MAX, "Invalid value for give type");
    }
    else if (mMember.mType == GetType<int32>())
    {
        NFE_ASSERT(max >= INT32_MIN && max <= INT32_MAX, "Invalid value for give type");
    }

    NFE_ASSERT(0 == (mMember.mMetadata.flags & MemberMetadata_HasMaxRange), "Property %s already has max range", mMember.GetName());
    mMember.mMetadata.flags = MemberMetadataFlags(mMember.mMetadata.flags | MemberMetadata_HasMaxRange);

    NFE_ASSERT(max >= mMember.mMetadata.min, "Max must be greater than min");
    mMember.mMetadata.max = max;

    return *this;
}

MemberMetadataBuilder& MemberMetadataBuilder::LogScale(float power)
{
    NFE_ASSERT(Math::IsValid(power), "Invalid power value");

    if (mMember.mType != GetType<double>() && mMember.mType != GetType<float>())
    {
        NFE_FATAL("Given property is not floating point type");
    }

    mMember.mMetadata.logScalePower = power;

    return *this;
}

MemberMetadataBuilder& MemberMetadataBuilder::NonNull()
{
    mMember.mMetadata.nonNull = true;

    if (mMember.mType->GetKind() != TypeKind::UniquePtr && mMember.mType->GetKind() != TypeKind::SharedPtr)
    {
        NFE_FATAL("Given property is not pointer type");
    }

    return *this;
}

} // namespace RTTI
} // namespace NFE
