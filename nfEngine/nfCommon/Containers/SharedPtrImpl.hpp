/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Shared pointer declaration
 */

#pragma once

#include "SharedPtr.hpp"


namespace NFE {
namespace Common {


template<typename T>
SharedPtr<T>::SharedPtr(T* ptr, const DeleterFunc& deleter)
    : SharedPtrBase(ptr, nullptr)
{
    if (mPointer)
    {
        mData = new SharedBlockType(ptr, deleter);
    }
}

template<typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& rhs)
    : SharedPtrBase(rhs.mPointer, rhs.mData)
{
    if (mData)
    {
        mData->AddWeakRef();
        mData->AddStrongRef();
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
    : SharedPtr(rhs.Release()) // TODO copy deleter from UniquePtr
{ }

template<typename T>
template<typename SourceType>
SharedPtr<T>::SharedPtr(const SharedPtr<SourceType>& rhs)
    : SharedPtrBase(rhs.mPointer, rhs.mData)
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
SharedPtr<T>& SharedPtr<T>::operator = (UniquePtr<T>&& rhs)
{
    // TODO copy deleter from UniquePtr
    Reset(rhs.Release());
    return *this;
}

template<typename T>
template<typename SourceType>
SharedPtr<T>& SharedPtr<T>::operator = (const SharedPtr<SourceType>& rhs)
{
    *this = StaticCast<T>(rhs); // TODO
    return *this;
}

template<typename T>
void SharedPtr<T>::Reset(T* newPtr, const DeleterFunc& deleter)
{
    if (mData)
    {
        SharedBlockType* data = static_cast<SharedBlockType*>(mData);
        T* ptr = data->GetPointer();
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
        mData = new SharedBlockType(newPtr, deleter);
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

//////////////////////////////////////////////////////////////////////////

template<typename T, typename ... Args>
SharedPtr<T> MakeSharedPtr(Args&& ... args)
{
    // TODO single allocation for both object and control block
    return SharedPtr<T>(new T(std::forward<Args>(args) ...));
}

template<typename TargetType, typename SourceType>
SharedPtr<TargetType> StaticCast(const SharedPtr<SourceType>& source)
{
    SharedPtr<TargetType> result;
    result.mData = source.mData;
    result.mPointer = static_cast<TargetType*>(source.mPointer);

    if (result.mData)
    {
        result.mData->AddWeakRef();
        result.mData->AddStrongRef();
    }

    return result;
}

template<typename TargetType, typename SourceType>
SharedPtr<TargetType> DynamicCast(const SharedPtr<SourceType>& source)
{
    SharedPtr<TargetType> result;
    result.mData = source.mData;
    result.mPointer = dynamic_cast<TargetType*>(source.mPointer);

    if (result.mData)
    {
        result.mData->AddWeakRef();
        result.mData->AddStrongRef();
    }

    return result;
}

} // namespace Common
} // namespace NFE
