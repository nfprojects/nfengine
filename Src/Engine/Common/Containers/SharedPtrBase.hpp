/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Shared pointer declaration
 */

#pragma once

#include "UniquePtr.hpp"
#include "SharedPtrData.hpp"


namespace NFE {
namespace Common {


template<typename T>
class WeakPtr;

/**
 * Common base for SharedPtr and WeakPtr.
 */
template<typename T>
class SharedPtrBase
{
public:
    NFE_FORCE_INLINE SharedPtrBase()
        : mPointer(nullptr)
        , mData(nullptr)
    { }

    NFE_FORCE_INLINE explicit SharedPtrBase(T* pointer, SharedPtrDataBase* data)
        : mPointer(pointer)
        , mData(data)
    { }

    /**
     * Get number of "strong" object references (number SharedPtr objects pointing to the target).
     */
    uint32 RefCount() const;

    /**
     * Get number of "weak" references (total number of SharedPtr and WeakPtr objects pointing to the target).
     */
    uint32 WeakRefCount() const;

protected:

    // Pointed object (for fast access).
    // Note that this pointer may be different than mData->GetPointer(), for example after casting to parent
    // class type.
    T* mPointer;

    // shared control block
    SharedPtrDataBase* mData;
};


template<typename T>
uint32 SharedPtrBase<T>::RefCount() const
{
    if (mData)
    {
        const uint32 numRefs = mData->mStrongRefs;
        NFE_ASSERT(numRefs >= 0, "Invalid ref count");
        return static_cast<uint32>(numRefs);
    }

    return 0;
}

template<typename T>
uint32 SharedPtrBase<T>::WeakRefCount() const
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
