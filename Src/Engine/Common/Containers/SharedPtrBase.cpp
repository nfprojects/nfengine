/**
 * @file
 * @author  Witek902
 * @brief   Shared pointer data
 */

#include "PCH.hpp"
#include "SharedPtrBase.hpp"


namespace NFE {
namespace Common {


uint32 SharedPtrBase::RefCount() const
{
    if (mData)
    {
        const int32 numRefs = mData->mStrongRefs;
        NFE_ASSERT(numRefs >= 0, "Invalid ref count");
        return static_cast<uint32>(numRefs);
    }

    return 0;
}

uint32 SharedPtrBase::WeakRefCount() const
{
    if (mData)
    {
        const uint32 numRefs = mData->mWeakRefs;
        NFE_ASSERT(numRefs > 0, "Invalid ref count");
        return static_cast<uint32>(numRefs);
    }

    return 0;
}

} // namespace Common
} // namespace NFE
