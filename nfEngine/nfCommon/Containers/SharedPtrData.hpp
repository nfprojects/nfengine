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
template<typename T>
class SharedPtrData final
{
public:
    SharedPtrData(T* pointer)
        : mPointer(pointer)
        , mStrongRefs(1)
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
        return (mStrongRefs--) == 1;
    }

    bool DelWeakRef()
    {
        return (mWeakRefs--) == 1;
    }

    uint32 GetNumStrongRefs() const
    {
        return mStrongRefs;
    }

    uint32 GetNumWeakRefs() const
    {
        return mStrongRefs;
    }

    T* GetPointer() const
    {
        return mPointer;
    }

private:
    T* mPointer;
    std::atomic<uint32> mStrongRefs;
    std::atomic<uint32> mWeakRefs;
};


} // namespace Common
} // namespace NFE
