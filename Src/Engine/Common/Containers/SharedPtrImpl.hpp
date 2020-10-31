/**
 * @file
 * @author  Witek902
 * @brief   Shared pointer declaration
 */

#pragma once

#include "SharedPtr.hpp"
#include "../Memory/DefaultAllocator.hpp"


namespace NFE {
namespace Common {


template<typename T>
SharedPtr<T>::SharedPtr(T* ptr, const DeleterFunc& deleter)
    : SharedPtrTypedBase<T>(ptr, nullptr)
{
    if (this->mPointer)
    {
        this->mData = new SharedBlockType(ptr, deleter);
    }
}

template<typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& rhs)
    : SharedPtrTypedBase<T>(rhs.mPointer, rhs.mData)
{
    if (this->mData)
    {
        this->mData->mWeakRefs++;
        this->mData->mStrongRefs++;
    }
}

template<typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& rhs)
{
    this->mPointer = rhs.mPointer;
    this->mData = rhs.mData;

    rhs.mPointer = nullptr;
    rhs.mData = nullptr;
}

template<typename T>
template<typename SourceType, typename SourceTypeDeleter>
SharedPtr<T>::SharedPtr(UniquePtr<SourceType, SourceTypeDeleter>&& rhs)
    : SharedPtr(
        rhs.ReleaseOwnership(),
        [] (T* ptr) { SourceTypeDeleter::Delete(static_cast<SourceType*>(ptr)); }
    )
{ }

template<typename T>
template<typename SourceType>
SharedPtr<T>::SharedPtr(const SharedPtr<SourceType>& rhs)
    : SharedPtrTypedBase<T>(rhs.mPointer, rhs.mData)
{
    if (this->mData)
    {
        this->mData->mWeakRefs++;
        this->mData->mStrongRefs++;
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

        this->mData = rhs.mData;
        this->mPointer = rhs.mPointer;

        if (this->mData)
        {
            this->mData->mStrongRefs++;
            this->mData->mWeakRefs++;
        }
    }

    return *this;
}

template<typename T>
SharedPtr<T>& SharedPtr<T>::operator = (SharedPtr&& rhs)
{
    Reset();

    this->mPointer = rhs.mPointer;
    this->mData = rhs.mData;

    rhs.mPointer = nullptr;
    rhs.mData = nullptr;

    return *this;
}

template<typename T>
template<typename SourceType, typename SourceTypeDeleter>
SharedPtr<T>& SharedPtr<T>::operator = (UniquePtr<SourceType, SourceTypeDeleter>&& rhs)
{
    const auto deleterFunc = [] (T* ptr) { SourceTypeDeleter::Delete(static_cast<SourceType*>(ptr)); };
    Reset(rhs.ReleaseOwnership(), deleterFunc);
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
    if (this->mData)
    {
        SharedBlockType* data = static_cast<SharedBlockType*>(this->mData);
        T* ptr = data->GetPointer();
        this->mData = nullptr;
        this->mPointer = nullptr;

        const int32 strongRefsBefore = data->mStrongRefs--;
        const int32 weakRefsBefore = data->mWeakRefs--;

        NFE_ASSERT(strongRefsBefore > 0, "Strong references counter underflow");
        NFE_ASSERT(weakRefsBefore > 0, "Weak references counter underflow");

        if (strongRefsBefore == 1)
        {
            const auto& oldObjectDeleter = data->GetDeleter();
            oldObjectDeleter(ptr);
        }

        if (weakRefsBefore == 1)
        {
            delete data;
        }
    }

    if (newPtr)
    {
        this->mPointer = newPtr;
        this->mData = new SharedBlockType(newPtr, deleter);
        // TODO possible optimization: reuse SharedPtrData
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
T* SharedPtr<T>::operator->() const
{
    return this->mPointer;
}

template<typename T>
T& SharedPtr<T>::operator*() const
{
    return *(this->mPointer);
}

template<typename T>
T* SharedPtr<T>::Get() const
{
    return this->mPointer;
}

template<typename T>
SharedPtr<T>::operator bool() const
{
    return this->mPointer != nullptr;
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
bool SharedPtr<T>::operator == (std::nullptr_t) const
{
    return this->mPointer == nullptr;
}

template<typename T>
bool SharedPtr<T>::operator != (std::nullptr_t) const
{
    return this->mPointer != nullptr;
}

template<typename T>
bool SharedPtr<T>::operator == (const SharedPtr& other) const
{
    return this->mPointer == other.mPointer;
}

template<typename T>
bool SharedPtr<T>::operator != (const SharedPtr& other) const
{
    return this->mPointer != other.mPointer;
}

template<typename T>
bool SharedPtr<T>::operator == (const T* other) const
{
    return this->mPointer == other;
}

template<typename T>
bool SharedPtr<T>::operator != (const T* other) const
{
    return this->mPointer != other;
}

template<typename T>
bool operator == (const T* lhs, const SharedPtr<T>& rhs)
{
    return rhs == lhs;
}

template<typename T>
bool operator != (const T* lhs, const SharedPtr<T>& rhs)
{
    return rhs != lhs;
}

//////////////////////////////////////////////////////////////////////////

template<typename T, typename ... Args>
SharedPtr<T> MakeSharedPtr(Args&& ... args)
{
    void* memory = NFE_MALLOC(sizeof(T), alignof(T));
    T* ptr = new (memory) T(std::forward<Args>(args) ...);
    return SharedPtr<T>(ptr);
}

template<typename TargetType, typename SourceType>
SharedPtr<TargetType> StaticCast(const SharedPtr<SourceType>& source)
{
    SharedPtr<TargetType> result;
    result.mData = source.mData;
    result.mPointer = static_cast<TargetType*>(source.mPointer);

    if (result.mData)
    {
        result.mData->mWeakRefs++;
        result.mData->mStrongRefs++;
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
        result.mData->mWeakRefs++;
        result.mData->mStrongRefs++;
    }

    return result;
}

template<typename T>
uint32 GetHash(const SharedPtr<T>& x)
{
    return GetHash(x.Get());
}


} // namespace Common
} // namespace NFE
