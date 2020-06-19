/**
 * @file
 * @author  Witek902
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
class NFE_ALIGN(8) SharedPtrData
{
public:
    using RefCountType = int32;

    NFCOMMON_API SharedPtrData();

    NFCOMMON_API virtual ~SharedPtrData();

    std::atomic<RefCountType> mStrongRefs;
    std::atomic<RefCountType> mWeakRefs;
};

/**
 * Shared pointer control data block - contains reference counters and deleter.
 */
template<typename T>
class SharedPtrTypedData final : public SharedPtrData
{
public:
    using DeleterFunc = std::function<void(T*)>;

    NFE_FORCE_INLINE SharedPtrTypedData(T* pointer, const DeleterFunc& deleter)
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
            if (object)
            {
                object->~T();
                NFE_FREE(object);
            }
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
