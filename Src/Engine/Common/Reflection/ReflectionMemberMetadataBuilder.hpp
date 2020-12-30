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

    // make normalized integer (0...1 or -1...1 range)
    MemberMetadataBuilder& Norm();

    MemberMetadataBuilder& LogScale(float power);

    // set custom debug name
    MemberMetadataBuilder& Name(const char* name);

    // force non-null pointer
    MemberMetadataBuilder& NonNull();

private:
    MemberMetadataBuilder(Member& member) : mMember(member) { }

    Member& mMember;
};


} // namespace RTTI
} // namespace NFE
