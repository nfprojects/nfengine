/**
 * @file
 * @author  Witek902
 * @brief   Shared pointer data
 */

#include "PCH.hpp"
#include "SharedPtrData.hpp"


namespace NFE {
namespace Common {


SharedPtrData::SharedPtrData()
    : mStrongRefs(1)
    , mWeakRefs(1)
{ }

SharedPtrData::~SharedPtrData()
{
    NFE_ASSERT(mStrongRefs == 0, "Strong references counter expected to be equal to zero");
    NFE_ASSERT(mWeakRefs == 0, "Weak references counter expected to be equal to zero");
}

} // namespace Common
} // namespace NFE
