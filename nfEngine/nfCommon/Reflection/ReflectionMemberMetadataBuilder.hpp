#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace RTTI {

class NFCOMMON_API MemberMetadataBuilder
{
    friend class Member;

public:
    ~MemberMetadataBuilder();

    // range specifier
    MemberMetadataBuilder& Min(double min);
    MemberMetadataBuilder& Max(double max);

    MemberMetadataBuilder& LogScale(float power);

    // set custom debug name
    MemberMetadataBuilder& Name(const char* name);

private:
    MemberMetadataBuilder(Member& member) : mMember(member) { }

    Member& mMember;
};


} // namespace RTTI
} // namespace NFE
