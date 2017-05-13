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
{ }

template<typename T>
SharedPtr<T>::SharedPtr(nullptr_t)
    : mPointer(nullptr)
    , mData(nullptr)
{ }

template<typename T>
SharedPtr<T>::SharedPtr(T* ptr)
    : mPointer(ptr)
    , mData(nullptr)
{
    if (mPointer)
    {
        mData = new SharedBlock(ptr);
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
void SharedPtr<T>::Reset(T* newPtr)
{
    if (mData)
    {
        SharedBlock* data = mData;
        T* ptr = mPointer;
        mData = nullptr;
        mPointer = nullptr;

        // TODO is this order OK?

        if (data->DelStrongRef())
        {
            // TODO custom deleter
            delete ptr;
        }

        if (data->DelWeakRef())
        {
            delete data;
        }
    }

    if (newPtr)
    {
        mPointer = newPtr;
        mData = new SharedBlock(newPtr);
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
bool SharedPtr<T>::operator == (const SharedPtr& other) const
{
    return mPointer == other.mPointer;
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

//////////////////////////////////////////////////////////////////////////

template<typename T, typename ... Args>
SharedPtr<T> MakeSharedPtr(Args&& ... args)
{
    // TODO single allocation for both object and control block
    return SharedPtr<T>(new T(std::forward<Args>(args) ...));
}

template<typename T, typename U>
SharedPtr<T> StaticCast(const SharedPtr<U>& source)
{
    SharedPtr<T> result;
    result.mData = source.mData;
    result.mPointer = static_cast<T*>(source.mPointer);

    if (result.mData)
    {
        result.mData->AddStrongRef();
        result.mData->AddWeakRef();
    }

    return result;
}

} // namespace Common
} // namespace NFE
