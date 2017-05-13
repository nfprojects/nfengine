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
    : SharedPtr(nullptr)
{ }

template<typename T>
SharedPtr<T>::SharedPtr(nullptr_t)
    : mPointer(nullptr)
    , mData(nullptr)
{ }

template<typename T>
SharedPtr<T>::SharedPtr(T* ptr, const DeleterFunc& deleter)
    : mPointer(ptr)
    , mData(nullptr)
{
    if (mPointer)
    {
        mData = new SharedBlock(ptr, deleter);
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
SharedPtr<T>::SharedPtr(UniquePtr<T>&& rhs)
    : SharedPtr(rhs.Release())
{ }

template<typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& rhs)
    : mPointer(rhs.mPointer)
    , mData(rhs.mData)
{
    if (mData)
    {
        mData->AddWeakRef();
        mData->AddStrongRef();
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
            mData->AddStrongRef();
            mData->AddWeakRef();
        }
    }

    return *this;
}

template<typename T>
void SharedPtr<T>::Reset(T* newPtr, const DeleterFunc& deleter)
{
    if (mData)
    {
        SharedBlock* data = static_cast<SharedBlock*>(mData);
        T* ptr = mPointer;
        mData = nullptr;
        mPointer = nullptr;

        if (data->DelStrongRef())
        {
            const DeleterFunc& oldObjectDeleter = data->GetDeleter();
            oldObjectDeleter(ptr);
        }

        if (data->DelWeakRef())
        {
            delete data;
        }
    }

    if (newPtr)
    {
        mPointer = newPtr;
        mData = new SharedBlock(newPtr, deleter);
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
bool SharedPtr<T>::operator != (const SharedPtr& other) const
{
    return mPointer != other.mPointer;
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

template<typename T>
uint32 SharedPtr<T>::WeakRefCount() const
{
    if (mData)
    {
        return mData->GetNumWeakRefs();
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
        result.mData->AddWeakRef();
        result.mData->AddStrongRef();
    }

    return result;
}

template<typename T, typename U>
SharedPtr<T> ReinterpretCast(const SharedPtr<U>& source)
{
    SharedPtr<T> result;
    result.mData = source.mData;
    result.mPointer = reinterpret_cast<T*>(source.mPointer);

    if (result.mData)
    {
        result.mData->AddWeakRef();
        result.mData->AddStrongRef();
    }

    return result;
}

} // namespace Common
} // namespace NFE
