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
    SharedPtrBase()
        : mPointer(nullptr)
        , mData(nullptr)
    { }

    explicit SharedPtrBase(T* pointer, SharedPtrDataBase* data)
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
        return mData->GetNumStrongRefs();
    }

    return 0;
}

template<typename T>
uint32 SharedPtrBase<T>::WeakRefCount() const
{
    if (mData)
    {
        return mData->GetNumWeakRefs();
    }

    return 0;
}


} // namespace Common
} // namespace NFE
