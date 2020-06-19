/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Unique pointer definitions
 */

#pragma once

#include "UniquePtr.hpp"
#include "../System/Assertion.hpp"
#include "../Memory/DefaultAllocator.hpp"


namespace NFE {
namespace Common {


template<typename T, typename Deleter>
UniquePtrBase<T, Deleter>::UniquePtrBase()
    : mPointer(nullptr)
{}

template<typename T, typename Deleter>
UniquePtrBase<T, Deleter>::UniquePtrBase(T* ptr)
    : mPointer(ptr)
{}

template<typename T, typename Deleter>
UniquePtrBase<T, Deleter>::UniquePtrBase(UniquePtrBase&& rhs)
    : mPointer(rhs.mPointer)
{
    rhs.mPointer = nullptr;
}

template<typename T, typename Deleter>
UniquePtrBase<T, Deleter>::~UniquePtrBase()
{
    Reset();
}

template<typename T, typename Deleter>
UniquePtrBase<T, Deleter>& UniquePtrBase<T, Deleter>::operator = (T* ptr)
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
UniquePtrBase<T, Deleter>& UniquePtrBase<T, Deleter>::operator = (UniquePtrBase&& ptr)
{
    Reset();
    mPointer = ptr.mPointer;
    ptr.mPointer = nullptr;
    return *this;
}

template<typename T, typename Deleter>
T* UniquePtrBase<T, Deleter>::Get() const
{
    return mPointer;
}

template<typename T, typename Deleter>
T** UniquePtrBase<T, Deleter>::GetPtr()
{
    return &mPointer;
}

template<typename T, typename Deleter>
void UniquePtrBase<T, Deleter>::Reset(T* newPtr)
{
    Deleter::Delete(mPointer);
    mPointer = newPtr;
}

template<typename T, typename Deleter>
T* UniquePtrBase<T, Deleter>::ReleaseOwnership()
{
    T* ptr = mPointer;
    mPointer = nullptr;
    return ptr;
}

template<typename T, typename Deleter>
UniquePtrBase<T, Deleter>::operator bool() const
{
    return mPointer != nullptr;
}

template<typename T, typename Deleter>
bool UniquePtrBase<T, Deleter>::operator == (const T* other) const
{
    return mPointer == other;
}

template<typename T, typename Deleter>
bool UniquePtrBase<T, Deleter>::operator != (const T* other) const
{
    return mPointer != other;
}

template<typename T, typename Deleter>
bool UniquePtrBase<T, Deleter>::operator == (const UniquePtrBase& other) const
{
    if (this == &other)
    {
        return true;
    }
    else
    {
        NFE_ASSERT(mPointer != other.mPointer, "Two different unique pointers must point to different objects. Fix your code.");
        return mPointer == other.mPointer;
    }
}

template<typename T, typename Deleter>
bool UniquePtrBase<T, Deleter>::operator != (const UniquePtrBase& other) const
{
    if (this == &other)
    {
        NFE_ASSERT(mPointer != other.mPointer, "Two different unique pointers must point to different objects. Fix your code.");
        return mPointer != other.mPointer;
    }
    else
    {
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename T, typename Deleter>
T* UniquePtr<T, Deleter>::operator->() const
{
    return this->mPointer;
}

template<typename T, typename Deleter>
T& UniquePtr<T, Deleter>::operator*() const
{
    return *(this->mPointer);
}

template<typename T, typename Deleter>
template<typename U>
UniquePtr<T, Deleter>::operator UniquePtr<U>()
{
    static_assert(std::is_base_of<U, T>::value, "Cannot convert unique pointer");

    UniquePtr<U> result(static_cast<U*>(this->mPointer));
    this->mPointer = nullptr;
    return result;
}

//////////////////////////////////////////////////////////////////////////

template<typename T, typename Deleter>
T& UniquePtr<T[], Deleter>::operator [] (size_t i) const
{
    NFE_ASSERT(this->mPointer, "Trying to access null array");
    return this->mPointer[i];
}

template<typename T, typename Deleter>
template<typename U>
UniquePtr<T[], Deleter>::operator UniquePtr<U[]>()
{
    static_assert(std::is_base_of<U, T>::value, "Cannot convert unique pointer");

    UniquePtr<U[]> result(static_cast<U*>(this->mPointer));
    this->mPointer = nullptr;
    return result;
}

//////////////////////////////////////////////////////////////////////////

template<typename T, typename... Args>
std::enable_if_t<!std::is_array<T>::value, UniquePtr<T>> MakeUniquePtr(Args&& ... args)
{
    void* memory = NFE_MALLOC(sizeof(T), alignof(T));
    T* ptr = new (memory) T(std::forward<Args>(args) ...);
    return UniquePtr<T>(ptr);
}

template<typename T>
std::enable_if_t<detail::is_unbounded_array_v<T>, UniquePtr<T>> MakeUniquePtr(size_t n)
{
    return UniquePtr<T>(new std::remove_extent_t<T>[n]());
}

//////////////////////////////////////////////////////////////////////////

template<typename T, typename U>
UniquePtr<T> StaticCast(UniquePtr<U>&& source)
{
    T* pointer = static_cast<T*>(source.ReleaseOwnership());
    return UniquePtr<T>(pointer);
}

template<typename T>
uint32 GetHash(const UniquePtr<T>& x)
{
    return GetHash(x.Get());
}


} // namespace Common
} // namespace NFE
