/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Shared pointer declaration
 */

#pragma once

#include "SharedPtr.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Common {


template<typename T>
SharedPtr<T>::SharedPtr()
    : mPointer(nullptr)
    , mData(nullptr)
{
}

template<typename T>
SharedPtr<T>::SharedPtr(T* ptr)
    : mPointer(ptr)
{
    if (mPointer)
    {
        mData = new SharedPtrData;
    }
}

template<typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& rhs)
{
    mPointer = rhs.mPointer;
    mData = rhs.mData;

    rhs.mPointer = nullptr;
    rhs.mData = nullptr;
}

template<typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& rhs)
    : mPointer(rhs.mPointer)
    , mData(rhs.mData)
{
    if (mData)
    {
        // TODO is this order OK?
        mData->AddStrongRef();
        mData->AddWeakRef();
    }
}

template<typename T>
SharedPtr<T>::~SharedPtr()
{
    Reset();
}

template<typename T>
SharedPtr<T>& SharedPtr<T>::operator = (SharedPtr&& rhs)
{
    Reset();

    mPointer = rhs.mPointer;
    mData = rhs.mData;

    rhs.mPointer = nullptr;
    rhs.mData = nullptr;

    return *this;
}

template<typename T>
SharedPtr<T>& SharedPtr<T>::operator = (const SharedPtr& rhs)
{
    if (&rhs != this)
    {
        Reset();

        mData = rhs.mData;
        mPointer = rhs.mPointer;

        if (mData)
        {
            // TODO is this order OK?
            mData->AddStrongRef();
            mData->AddWeakRef();
        }
    }

    return *this;
}

template<typename T>
void SharedPtr<T>::Reset(T* newPtr = nullptr)
{
    if (mData)
    {
        // TODO is this order OK?

        if (mData->DelStrongRef())
        {
            // TODO custom deleter
            delete mPointer;
        }

        if (mData->DelWeakRef()) {
            delete mData;
        }

        mData = nullptr;
        mPointer = nullptr;
    }

    if (newPtr)
    {
        mPointer = newPtr;
        mData = new SharedPtrData();
        // TODO possible optimization: reuse SharedPtrData
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
T** SharedPtr<T>::operator&()
{
    return &mPointer;
}

template<typename T>
T* SharedPtr<T>::operator->() const
{
    return mPointer;
}

template<typename T>
T& SharedPtr<T>::operator*() const
{
    return *mPointer;
}

template<typename T>
T* SharedPtr<T>::Get() const
{
    return mPointer;
}

template<typename T>
SharedPtr<T>::operator bool() const
{
    return mPointer != nullptr;
}

template<typename T>
bool SharedPtr<T>::operator == (const T* other) const
{
    return mPointer == other;
}

template<typename T>
bool SharedPtr<T>::operator != (const T* other) const
{
    return mPointer != other;
}

template<typename T>
uint32 SharedPtr<T>::RefCount() const
{
    if (mData)
    {
        return mData->GetNumStrongRefs();
    }

    return 0;
}

} // namespace Common
} // namespace NFE
