/**
 * @file
 * @author Witek902
 * @brief  Dynamic array container definitions
 */

#pragma once

#include "Deque.hpp"
#include "../System/Assertion.hpp"
#include "../Math/Math.hpp"
#include "../Memory/MemoryHelpers.hpp"


namespace NFE {
namespace Common {

template<typename ElementType>
Deque<ElementType>::Deque()
    : mElements(nullptr)
    , mCapacity(0)
    , mHead(0)
    , mSize(0)
{
}

template<typename ElementType>
Deque<ElementType>::Deque(const uint32 intialCapacity)
    : mElements(nullptr)
    , mCapacity(0)
    , mHead(0)
    , mSize(0)
{
    Reserve(intialCapacity);
}

template<typename ElementType>
Deque<ElementType>::~Deque()
{
    Clear(true);
}

template<typename ElementType>
Deque<ElementType>::Deque(const Deque& other)
    : mElements(nullptr)
    , mCapacity(0)
    , mHead(0)
    , mSize(0)
{
    if (!Reserve(other.mSize))
    {
        NFE_LOG_ERROR("Failed to reserve memory for Deque");
        return;
    }

    this->mSize = other.mSize;
    for (uint32 i = 0; i < other.mSize; ++i)
    {
        new (this->mElements + i) ElementType(other.mElements[i]);
    }
}

template<typename ElementType>
Deque<ElementType>::Deque(Deque&& other)
    : mElements(nullptr)
    , mCapacity(0)
    , mHead(0)
    , mSize(0)
{
    // don't free memory if not needed
    Clear(true);

    this->mElements = other.mElements;
    this->mSize = other.mSize;
    mCapacity = other.mCapacity;

    other.mElements = nullptr;
    other.mSize = 0;
    other.mCapacity = 0;
}

template<typename ElementType>
Deque<ElementType>& Deque<ElementType>::operator = (const Deque& other)
{
    if (&other == this)
    {
        return *this;
    }

    Clear();

    if (!Reserve(other.mSize))
    {
        NFE_LOG_ERROR("Failed to reserve memory for Deque");
        return *this;
    }

    this->mSize = other.mSize;
    for (uint32 i = 0; i < other.mSize; ++i)
    {
        const Element& otherElement = other.mElements[other.GetElementIndex(i)];
        new (this->mElements + i) ElementType(otherElement);
    }

    return *this;
}

template<typename ElementType>
Deque<ElementType>& Deque<ElementType>::operator = (Deque&& other)
{
    // don't free memory if not needed
    Clear(true);

    this->mElements = other.mElements;
    this->mSize = other.mSize;
    mCapacity = other.mCapacity;

    other.mElements = nullptr;
    other.mSize = 0;
    other.mCapacity = 0;

    return *this;
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
uint32 Deque<ElementType>::Size() const
{
    return mSize;
}

template<typename ElementType>
bool Deque<ElementType>::Empty() const
{
    return mSize == 0;
}

template<typename ElementType>
uint32 Deque<ElementType>::GetElementIndex(uint32 index) const
{
    return (mHead + index) & (mCapacity - 1);
}

template<typename ElementType>
void Deque<ElementType>::Clear(bool freeMemory)
{
    // call destructors
    for (uint32 i = 0; i < this->mSize; ++i)
    {
        this->mElements[GetElementIndex(i)].~ElementType();
    }

    this->mSize = 0;

    if (freeMemory)
    {
        NFE_FREE(this->mElements);
        this->mElements = nullptr;
        mCapacity = 0;
    }
}

template<typename ElementType>
bool Deque<ElementType>::ContainsElement(const ElementType& element) const
{
    // TODO
    (void)element;

    return false;
}

template<typename ElementType>
const ElementType& Deque<ElementType>::Front() const
{
    NFE_ASSERT(mSize != 0, "Trying to access an element of an empty deque");
    return mElements[mHead];
}

template<typename ElementType>
ElementType& Deque<ElementType>::Front()
{
    NFE_ASSERT(mSize != 0, "Trying to access an element of an empty deque");
    return mElements[mHead];
}

template<typename ElementType>
const ElementType& Deque<ElementType>::Back() const
{
    NFE_ASSERT(mSize != 0, "Trying to access an element of an empty deque");
    return mElements[GetElementIndex(mSize - 1)];
}

template<typename ElementType>
ElementType& Deque<ElementType>::Back()
{
    NFE_ASSERT(mSize != 0, "Trying to access an element of an empty deque");
    return mElements[GetElementIndex(mSize - 1)];
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
bool Deque<ElementType>::PushBack(const ElementType& element)
{
    NFE_ASSERT(!ContainsElement(element), "Adding element to a Deque that is already contained by the array is not supported");

    if (!Reserve(this->mSize + 1))
    {
        return false;
    }

    new (this->mElements + GetElementIndex(this->mSize)) ElementType(element);
    this->mSize++;

    return true;
}

template<typename ElementType>
bool Deque<ElementType>::PushBack(ElementType&& element)
{
    NFE_ASSERT(!ContainsElement(element), "Adding element to a Deque that is already contained by the array is not supported");

    if (!Reserve(this->mSize + 1))
    {
        return false;
    }

    new (this->mElements + GetElementIndex(this->mSize)) ElementType(std::move(element));
    this->mSize++;

    return true;
}

template<typename ElementType>
template<typename ... Args>
bool Deque<ElementType>::EmplaceBack(Args&& ... args)
{
    if (!Reserve(this->mSize + 1))
    {
        return false;
    }

    new (this->mElements + GetElementIndex(this->mSize)) ElementType(std::forward<Args>(args) ...);
    this->mSize++;

    return true;
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
bool Deque<ElementType>::PushFront(const ElementType& element)
{
    NFE_ASSERT(!ContainsElement(element), "Adding element to a Deque that is already contained by the array is not supported");

    if (!Reserve(this->mSize + 1))
    {
        return false;
    }

    const uint32 targetIndex = (mHead - 1) & (mCapacity - 1);
    new (this->mElements + targetIndex) ElementType(element);

    this->mSize++;
    this->mHead = targetIndex;
    return true;
}

template<typename ElementType>
bool Deque<ElementType>::PushFront(ElementType&& element)
{
    NFE_ASSERT(!ContainsElement(element), "Adding element to a Deque that is already contained by the array is not supported");

    if (!Reserve(this->mSize + 1))
    {
        return false;
    }

    const uint32 targetIndex = (mHead - 1) & (mCapacity - 1);
    new (this->mElements + targetIndex) ElementType(std::move(element));

    this->mSize++;
    this->mHead = targetIndex;
    return true;
}

template<typename ElementType>
template<typename ... Args>
bool Deque<ElementType>::EmplaceFront(Args&& ... args)
{
    NFE_ASSERT(!ContainsElement(element), "Adding element to a Deque that is already contained by the array is not supported");

    if (!Reserve(this->mSize + 1))
    {
        return false;
    }

    const uint32 targetIndex = (mHead - 1) & (mCapacity - 1);
    new (this->mElements + targetIndex) ElementType(std::forward<Args>(args) ...);

    this->mSize++;
    this->mHead = targetIndex;
    return true;
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
bool Deque<ElementType>::PopBack()
{
    if (this->Empty())
    {
        return false;
    }

    this->mElements[GetElementIndex(--this->mSize)].~ElementType();
    return true;
}

template<typename ElementType>
bool Deque<ElementType>::PopFront()
{
    if (this->Empty())
    {
        return false;
    }

    this->mElements[GetElementIndex(0)].~ElementType();

    mSize--;
    this->mHead = (this->mHead + 1) & (mCapacity - 1);

    return true;
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
bool Deque<ElementType>::Reserve(uint32 size)
{
    if (size <= mCapacity)
    {
        // smaller that allocated - ignore
        return true;
    }

    uint32 newAllocSize = mCapacity;
    while (size > newAllocSize)
    {
        // grow by 100%
        newAllocSize = Math::Max<uint32>(1, newAllocSize * 2);
    }

    ElementType* newBuffer = static_cast<ElementType*>(NFE_MALLOC(newAllocSize * sizeof(ElementType), alignof(ElementType)));
    if (!newBuffer)
    {
        // memory allocation failed
        return false;
    }

    // source buffer: aa....bbbb
    //                      ^ head
    // target buffer: bbbbaa...............
    //                ^ head
    const uint32 numElementBeforeBufferEnd = Math::Min<uint32>(mSize, mCapacity - mHead);
    MemoryHelpers::MoveArray<ElementType>(newBuffer, this->mElements + this->mHead, numElementBeforeBufferEnd);
    MemoryHelpers::MoveArray<ElementType>(newBuffer + numElementBeforeBufferEnd, this->mElements, mSize - numElementBeforeBufferEnd);

    // replace buffer
    NFE_FREE(this->mElements);
    this->mElements = newBuffer;
    this->mCapacity = newAllocSize;
    this->mHead = 0;

    return true;
}

template<typename ElementType>
void Deque<ElementType>::Swap(Deque& other)
{
    std::swap(mElements, other.mElements);
    std::swap(mCapacity, other.mCapacity);
    std::swap(mHead, other.mHead);
    std::swap(mSize, other.mSize);
}


} // namespace Common
} // namespace NFE
