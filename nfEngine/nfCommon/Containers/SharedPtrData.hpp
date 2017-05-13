/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Shared pointer control data block declaration
 */

#pragma once

#include "../nfCommon.hpp"
#include "../System/Assertion.hpp"

#include <atomic>
#include <functional>


namespace NFE {
namespace Common {


/**
* Shared pointer control data block - contains reference counters and deleter.
*/
class SharedPtrDataBase
{
public:
    SharedPtrDataBase()
        : mStrongRefs(1)
        , mWeakRefs(1)
    { }

    virtual ~SharedPtrDataBase()
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
        return mWeakRefs;
    }

protected:
    std::atomic<uint32> mStrongRefs;
    std::atomic<uint32> mWeakRefs;
};

/**
 * Shared pointer control data block - contains reference counters and deleter.
 */
template<typename T>
class SharedPtrData final : public SharedPtrDataBase
{
public:
    using DeleterFunc = std::function<void(T*)>;

    explicit SharedPtrData(T* pointer, const DeleterFunc& deleter)
        : mPointer(pointer)
        , mDeleter(deleter)
    { }

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

    // deleter function
    DeleterFunc mDeleter;
};


} // namespace Common
} // namespace NFE
