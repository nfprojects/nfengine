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
    this->mPointer = rhs.mPointer;
    this->mData = rhs.mData;

    rhs.mPointer = nullptr;
    rhs.mData = nullptr;
}

template<typename T>
WeakPtr<T>::WeakPtr(const WeakPtr<T>& rhs)
    : SharedPtrBase<T>(rhs.mPointer, rhs.mData)
{
    if (this->mData)
    {
        this->mData->AddWeakRef();
    }
}

template<typename T>
WeakPtr<T>::WeakPtr(const SharedPtr<T>& rhs)
    : SharedPtrBase<T>(rhs.mPointer, rhs.mData)
{
    if (this->mData)
    {
        this->mData->AddWeakRef();
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

    this->mPointer = rhs.mPointer;
    this->mData = rhs.mData;

    rhs.mPointer = nullptr;
    rhs.mData = nullptr;

    return *this;
}

template<typename T>
WeakPtr<T>& WeakPtr<T>::operator = (const SharedPtr<T>& rhs)
{
    if (this->mData != rhs.mData)
    {
        Reset();

        this->mData = rhs.mData;
        if (this->mData)
        {
            this->mPointer = rhs.mPointer;
            this->mData->AddWeakRef();
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

        this->mData = rhs.mData;
        if (this->mData)
        {
            this->mPointer = rhs.mPointer;
            this->mData->AddWeakRef();
        }
    }

    return *this;
}

template<typename T>
template<typename SourceType>
WeakPtr<T>::WeakPtr(const WeakPtr<SourceType>& rhs)
    : SharedPtrBase<T>(rhs.mPointer, rhs.mData)
{
    if (this->mData)
    {
        this->mData->AddWeakRef();
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
void WeakPtr<T>::Reset()
{
    if (this->mData)
    {
        if (this->mData->DelWeakRef())
        {
            delete this->mData;
        }

        this->mData = nullptr;
    }
}

template<typename T>
SharedPtr<T> WeakPtr<T>::Lock() const
{
    SharedPtr<T> result;

    // TODO there's race condition here
    if (this->mData && this->mData->GetNumStrongRefs() > 0)
    {
        this->mData->AddStrongRef();
        this->mData->AddWeakRef();

        result.mData = this->mData;
        result.mPointer = this->mPointer;
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

    return this->mData == other.mData;
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

    return this->mData == other.mData;
}

template<typename T>
bool operator == (const SharedPtr<T>& lhs, const WeakPtr<T>& rhs)
{
    // reuse WeakPtr<T>::operator ==
    return rhs == lhs;
}

template<typename T>
bool WeakPtr<T>::Valid() const
{
    if (this->mData)
    {
        return this->mData->GetNumStrongRefs() > 0;
    }

    return false;
}


} // namespace Common
} // namespace NFE
