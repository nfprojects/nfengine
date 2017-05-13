/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Weak pointer declaration
 */

#pragma once

#include "WeakPtr.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Common {


template<typename T>
WeakPtr<T>::WeakPtr(WeakPtr<T>&& rhs)
{
    mData = rhs.mData;
    rhs.mData = nullptr;
}

template<typename T>
WeakPtr<T>::WeakPtr(const WeakPtr<T>& rhs)
    : mData(rhs.mData)
{
    if (mData)
    {
        mData->AddWeakRef();
    }
}

template<typename T>
WeakPtr<T>::WeakPtr(const SharedPtr<T>& rhs)
    : mData(rhs.mData)
{
    if (mData)
    {
        mData->AddWeakRef();
    }
}

template<typename T>
WeakPtr<T>::~WeakPtr()
{
    Reset();
}

template<typename T>
WeakPtr<T>& WeakPtr<T>::operator = (WeakPtr<T>&& rhs)
{
    Reset();
    mData = rhs.mData;
    rhs.mData = nullptr;
    return *this;
}

template<typename T>
WeakPtr<T>& WeakPtr<T>::operator = (const SharedPtr<T>& rhs)
{
    if (mData != rhs.mData)
    {
        Reset();

        mData = rhs.mData;
        if (mData)
        {
            mData->AddWeakRef();
        }
    }

    return *this;
}

template<typename T>
WeakPtr<T>& WeakPtr<T>::operator = (const WeakPtr<T>& rhs)
{
    if (&rhs != this)
    {
        Reset();

        mData = rhs.mData;
        if (mData)
        {
            mData->AddWeakRef();
        }
    }

    return *this;
}

template<typename T>
template<typename SourceType>
WeakPtr<T>::WeakPtr(const WeakPtr<SourceType>& rhs)
    : mData(rhs.mData)
{
    if (mData)
    {
        mData->AddWeakRef();
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
void WeakPtr<T>::Reset()
{
    if (mData)
    {
        if (mData->DelWeakRef())
        {
            delete mData;
        }

        mData = nullptr;
    }
}

template<typename T>
SharedPtr<T> WeakPtr<T>::Lock() const
{
    SharedPtr<T> result;
    if (mData)
    {
        // TODO there's race condition here
        mData->AddStrongRef();
        mData->AddWeakRef();

        result.mData = mData;
        result.mPointer = GetSharedData()->GetPointer();
    }
    return result;
}

template<typename T>
bool WeakPtr<T>::operator == (const SharedPtr<T>& other) const
{
    const bool thisValid = Valid();
    const bool otherValid = other;

    if (!thisValid || !otherValid)
    {
        // if weak pointer is invalid, other pointer must be null or invalid too
        return thisValid == otherValid;
    }

    return mData == other.mData;
}

template<typename T>
bool WeakPtr<T>::operator == (const WeakPtr<T>& other) const
{
    const bool thisValid = Valid();
    const bool otherValid = other.Valid();

    if (!thisValid || !otherValid)
    {
        return thisValid == otherValid;
    }

    return mData == other.mData;
}

template<typename T>
bool operator == (const SharedPtr<T>& lhs, const WeakPtr<T>& rhs)
{
    // reuse WeakPtr<T>::operator ==
    return rhs == lhs;
}

template<typename T>
uint32 WeakPtr<T>::WeakRefCount() const
{
    if (mData)
    {
        return mData->GetNumWeakRefs();
    }

    return 0;
}

template<typename T>
bool WeakPtr<T>::Valid() const
{
    if (mData)
    {
        return mData->GetNumStrongRefs() > 0;
    }

    return false;
}


} // namespace Common
} // namespace NFE
