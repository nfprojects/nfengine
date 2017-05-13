/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Shared pointer control data block declaration
 */

#pragma once

#include "../nfCommon.hpp"

#include <atomic>


namespace NFE {
namespace Common {


/**
 * Shared pointer control data block - contains reference counters.
 */
class SharedPtrData final
{
public:
    SharedPtrData()
        : mStrongRefs(1)
        , mWeakRefs(1)
    { }

    SharedPtrData::~SharedPtrData()
    {
        NFE_ASSERT(mStrongRefs == 0, "Strong references counter expected to be equal to zero");
        NFE_ASSERT(mWeakRefs == 0, "Weak references counter expected to be equal to zero");
    }

    void AddStrongRef()
    {
        mStrongRefs++;
    }

    void AddWeakRef()
    {
        mWeakRefs++;
    }

    bool DelStrongRef()
    {
        return (--mStrongRefs) == 0;
    }

    bool DelWeakRef()
    {
        return (--mWeakRefs) == 0;
    }

    uint32 GetNumStrongRefs() const
    {
        return mStrongRefs;
    }

    uint32 GetNumWeakRefs() const
    {
        return mStrongRefs;
    }

private:
    std::atomic<uint32> mStrongRefs;
    std::atomic<uint32> mWeakRefs;
};


} // namespace Common
} // namespace NFE
