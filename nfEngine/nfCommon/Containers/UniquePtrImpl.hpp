/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unique mPointer declaration
 */

#pragma once

#include "UniquePtr.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Common {


template<typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr()
    : mPointer(nullptr)
{
}

template<typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(T* ptr)
    : mPointer(ptr)
{
}

template<typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(UniquePtr&& rhs)
    : mPointer(rhs.mPointer)
{
    rhs.mPointer = nullptr;
}

template<typename T, typename Deleter>
UniquePtr<T, Deleter>::~UniquePtr()
{
    Reset();
}

template<typename T, typename Deleter>
UniquePtr<T, Deleter>& UniquePtr<T, Deleter>::operator = (T* ptr)
{
    NFE_ASSERT(mPointer != ptr, "Assigning the same object to an unique pointer. Fix your code.");

    if (mPointer != ptr)
    {
        Deleter::Delete(mPointer);
        mPointer = ptr;
    }
    return *this;
}

template<typename T, typename Deleter>
UniquePtr<T, Deleter>& UniquePtr<T, Deleter>::operator = (UniquePtr&& ptr)
{
    Reset();
    mPointer = ptr.mPointer;
    ptr.mPointer = nullptr;
    return *this;
}

template<typename T, typename Deleter>
T** UniquePtr<T, Deleter>::operator&()
{
    return &mPointer;
}

template<typename T, typename Deleter>
T* UniquePtr<T, Deleter>::operator->() const
{
    return mPointer;
}

template<typename T, typename Deleter>
T& UniquePtr<T, Deleter>::operator*() const
{
    return *mPointer;
}

template<typename T, typename Deleter>
T* UniquePtr<T, Deleter>::Get() const
{
    return mPointer;
}

template<typename T, typename Deleter>
void UniquePtr<T, Deleter>::Reset(T* newPtr = nullptr)
{
    Deleter::Delete(mPointer);
    mPointer = newPtr;
}

template<typename T, typename Deleter>
T* UniquePtr<T, Deleter>::Release()
{
    T* ptr = mPointer;
    mPointer = nullptr;
    return ptr;
}

template<typename T, typename Deleter>
UniquePtr<T, Deleter>::operator bool() const
{
    return mPointer != nullptr;
}

template<typename T, typename Deleter>
bool UniquePtr<T, Deleter>::operator == (const T* other) const
{
    return mPointer == other;
}

template<typename T, typename Deleter>
bool UniquePtr<T, Deleter>::operator != (const T* other) const
{
    return mPointer != other;
}

template<typename T, typename ... Args>
UniquePtr<T> MakeUniquePtr(Args&& ... args)
{
    return UniquePtr<T>(new T(std::forward<Args>(args) ...));
}

} // namespace Common
} // namespace NFE
