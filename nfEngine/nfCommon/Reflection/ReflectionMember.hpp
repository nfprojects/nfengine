/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of reflection system's Member class.
 */

#pragma once

#include "../nfCommon.hpp"
#include "ReflectionMemberMetadataBuilder.hpp"
#include "../Utils/FundamentalTypesUnion.hpp"
#include "../Containers/String.hpp"

namespace NFE {
namespace RTTI {

enum MemberMetadataFlags
{
    MemberMetadata_HasMinRange  = 1 << 0,
    MemberMetadata_HasMaxRange  = 1 << 1,
    MemberMetadata_NoSlider     = 1 << 2,
    //MemberMetadata_ReadOnly     = 1 << 3,
};

struct MemberMetadata
{
    Common::String descriptiveName;

    // allowed values range
    double min = -DBL_MAX;
    double max = DBL_MAX;

    MemberMetadataFlags flags = MemberMetadataFlags(0);

    float logScalePower = 1.0f;

    bool HasMinRange() const { return (flags & MemberMetadata_HasMinRange) != 0; }
    bool HasMaxRange() const { return (flags & MemberMetadata_HasMaxRange) != 0; }
    bool HasNoSlider() const { return (flags & MemberMetadata_NoSlider) != 0; }
};

/**
 * Class member information.
 */
class NFCOMMON_API Member
{
    friend class MemberMetadataBuilder;

public:
    Member(const char* name, size_t offset, const Type* type);

    const char* GetName() const { return mName; }
    const char* GetDescriptiveName() const { return mMetadata.descriptiveName.Str(); }
    const Type* GetType() const { return mType; }
    uint32 GetOffset() const { return mOffset; }

    const MemberMetadata& GetMetadata() const { return mMetadata; }

    MemberMetadataBuilder GetMetadataBuilder()
    {
        return MemberMetadataBuilder(*this);
    }

private:
    const Type* mType;  // member type
    const char* mName;  // member name
    uint32 mOffset;     // offset from the beginning of the class (in bytes)

    MemberMetadata mMetadata; // used by editor
};

} // namespace RTTI
} // namespace NFE
