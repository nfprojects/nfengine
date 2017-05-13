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
    using DeleterFunc = std::function<void(T*)>;

    SharedPtrData(T* pointer, const DeleterFunc& deleter)
        : mPointer(pointer)
        , mStrongRefs(1)
        , mWeakRefs(1)
        , mDeleter(deleter)
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

    const DeleterFunc& GetDeleter() const
    {
        return mDeleter;
    }

    static DeleterFunc GetDefaultDeleter()
    {
        static const auto defaultDeleter = [](T* object)
        {
            delete object;
        };

        return defaultDeleter;
    }

private:
    T* mPointer;
    std::atomic<uint32> mStrongRefs;
    std::atomic<uint32> mWeakRefs;

    // deleter function
    DeleterFunc mDeleter;
};


} // namespace Common
} // namespace NFE
