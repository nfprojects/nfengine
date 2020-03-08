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
    NFE_FORCE_INLINE SharedPtrDataBase()
        : mStrongRefs(1)
        , mWeakRefs(1)
    { }

    NFE_FORCE_INLINE virtual ~SharedPtrDataBase()
    {
        NFE_ASSERT(mStrongRefs == 0, "Strong references counter expected to be equal to zero");
        NFE_ASSERT(mWeakRefs == 0, "Weak references counter expected to be equal to zero");
    }

    std::atomic<int32> mStrongRefs;
    std::atomic<int32> mWeakRefs;
};

/**
 * Shared pointer control data block - contains reference counters and deleter.
 */
template<typename T>
class SharedPtrData final : public SharedPtrDataBase
{
public:
    using DeleterFunc = std::function<void(T*)>;

    NFE_FORCE_INLINE explicit SharedPtrData(T* pointer, const DeleterFunc& deleter)
        : mPointer(pointer)
        , mDeleter(deleter)
    { }

    NFE_FORCE_INLINE T* GetPointer() const
    {
        return mPointer;
    }

    NFE_FORCE_INLINE const DeleterFunc& GetDeleter() const
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
