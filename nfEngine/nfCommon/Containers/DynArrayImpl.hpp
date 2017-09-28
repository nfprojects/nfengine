/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Dynamic array container definitions
 */

#pragma once

#include "DynArray.hpp"
#include "../System/Assertion.hpp"
#include "../Math/Math.hpp"
#include "../Memory/MemoryHelpers.hpp"


namespace NFE {
namespace Common {

template<typename ElementType>
DynArray<ElementType>::DynArray()
    : mAllocSize(0)
{
}

template<typename ElementType>
DynArray<ElementType>::~DynArray()
{
    Clear(true);
}

template<typename ElementType>
DynArray<ElementType>::DynArray(const DynArray& other)
    : ArrayView<ElementType>()
{
    mAllocSize = 0;
    this->mElements = nullptr;
    this->mSize = 0;

    if (!Reserve(other.mSize))
    {
        LOG_ERROR("Failed to reserve memory for DynArray");
        return;
    }

    this->mSize = other.mSize;
    for (uint32 i = 0; i < other.mSize; ++i)
    {
        new (this->mElements + i) ElementType(other.mElements[i]);
    }
}

template<typename ElementType>
DynArray<ElementType>::DynArray(DynArray&& other)
    : ArrayView<ElementType>()
{
    // don't free memory if not needed
    Clear(true);

    this->mElements = other.mElements;
    this->mSize = other.mSize;
    mAllocSize = other.mAllocSize;

    other.mElements = nullptr;
    other.mSize = 0;
    other.mAllocSize = 0;
}

template<typename ElementType>
typename DynArray<ElementType>::DynArray& DynArray<ElementType>::operator = (const DynArray& other)
{
    if (&other == this)
        return *this;

    Clear();

    if (!Reserve(other.mSize))
    {
        LOG_ERROR("Failed to reserve memory for DynArray");
        return *this;
    }

    this->mSize = other.mSize;
    for (uint32 i = 0; i < other.mSize; ++i)
    {
        new (this->mElements + i) ElementType(other.mElements[i]);
    }

    return *this;
}

template<typename ElementType>
typename DynArray<ElementType>::DynArray& DynArray<ElementType>::operator = (DynArray&& other)
{
    // don't free memory if not needed
    Clear(true);

    this->mElements = other.mElements;
    this->mSize = other.mSize;
    mAllocSize = other.mAllocSize;

    other.mElements = nullptr;
    other.mSize = 0;
    other.mAllocSize = 0;

    return *this;
}

template<typename ElementType>
DynArray<ElementType>::DynArray(const std::initializer_list<ElementType>& list)
    : DynArray()
{
    if (!Reserve(static_cast<uint32>(list.size())))
    {
        LOG_ERROR("Failed to reserve memory for DynArray");
        return;
    }

    for (const ElementType element : list)
    {
        PushBack(element);
    }
}

template<typename ElementType>
DynArray<ElementType>::DynArray(const ElementType* elements, uint32 count)
    : DynArray()
{
    if (!Reserve(count))
    {
        LOG_ERROR("Failed to reserve memory for DynArray");
        return;
    }

    this->mSize = count;
    for (uint32 i = 0; i < count; ++i)
    {
        new (this->mElements + i) ElementType(elements[i]);
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
void DynArray<ElementType>::Clear(bool freeMemory)
{
    if (!this->mElements)
    {
        // already empty
        return;
    }

    // call destructors
    for (uint32 i = 0; i < this->mSize; ++i)
    {
        this->mElements[i].~ElementType();
    }

    this->mSize = 0;

    if (freeMemory)
    {
        NFE_FREE(this->mElements);
        this->mElements = nullptr;
        mAllocSize = 0;
    }
}

template<typename ElementType>
typename DynArray<ElementType>::IteratorType DynArray<ElementType>::PushBack(const ElementType& element)
{
    if (!Reserve(this->mSize + 1))
    {
        return this->End();
    }

    new (this->mElements + this->mSize) ElementType(element);
    return IteratorType(this, this->mSize++);
}

template<typename ElementType>
typename DynArray<ElementType>::IteratorType DynArray<ElementType>::PushBack(ElementType&& element)
{
    if (!Reserve(this->mSize + 1))
    {
        // memory allocation failed
        return this->End();
    }

    new (this->mElements + this->mSize) ElementType(std::move(element));
    return IteratorType(this, this->mSize++);
}

template<typename ElementType>
template<typename ElementType2>
bool DynArray<ElementType>::PushBackArray(const ArrayView<ElementType2>& arrayView)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, typename std::remove_cv<ElementType2>::type>::value,
                  "Incompatible element types");

    if (arrayView.Empty())
    {
        // empty array - do nothing
        return true;
    }

    if (!Reserve(this->mSize + arrayView.mSize))
    {
        // memory allocation failed
        return false;
    }

    // copy elements
    for (uint32 i = 0; i < arrayView.mSize; ++i)
    {
        new (this->mElements + this->mSize + i) ElementType(arrayView.mElements[i]);
    }

    this->mSize += arrayView.mSize;
    return true;
}

template<typename ElementType>
bool DynArray<ElementType>::PopBack()
{
    if (this->Empty())
        return false;

    this->mElements[--this->mSize].~ElementType();
    return true;
}

template<typename ElementType>
typename DynArray<ElementType>::IteratorType DynArray<ElementType>::InsertAt(uint32 index, const ElementType& element)
{
    if (!Reserve(this->mSize + 1))
    {
        // memory allocation failed
        return this->End();
    }

    ElementType* base = this->mElements + index;
    memmove(base + 1, base, sizeof(ElementType) * (this->mSize - index));
    new (base) ElementType(element);
    this->mSize++;
    return IteratorType(this, index);
}

template<typename ElementType>
typename DynArray<ElementType>::IteratorType DynArray<ElementType>::InsertAt(uint32 index, ElementType&& element)
{
    if (!Reserve(this->mSize + 1))
    {
        // memory allocation failed
        return this->End();
    }

    ElementType* base = this->mElements + index;
    memmove(base + 1, base, sizeof(ElementType) * (this->mSize - index));
    new (base) ElementType(std::move(element));
    this->mSize++;
    return IteratorType(this, index);
}

template<typename ElementType>
typename DynArray<ElementType>::IteratorType DynArray<ElementType>::InsertAt(uint32 index, const ElementType& element, uint32 count)
{
    if (count == 0)
    {
        // nothing to DO
        return this->End();
    }

    if (!Reserve(this->mSize + count))
    {
        // memory allocation failed
        return this->End();
    }

    ElementType* base = this->mElements + index;
    memmove(base + count, base, sizeof(ElementType) * (this->mSize - index));

    for (uint32 i = 0; i < count; ++i)
    {
        new (base + i) ElementType(element);
    }

    this->mSize += count;
    return IteratorType(this, index);
}

template<typename ElementType>
template<typename ElementType2>
typename DynArray<ElementType>::IteratorType DynArray<ElementType>::InsertArrayAt(uint32 index, const ArrayView<ElementType2>& arrayView)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, typename std::remove_cv<ElementType2>::type>::value,
                  "Incompatible element types");

    if (arrayView.Empty())
    {
        // trying to insert empty array - do nothing
        return this->End();
    }

    if (!Reserve(this->mSize + arrayView.mSize))
    {
        // memory allocation failed
        return this->End();
    }

    ElementType* base = this->mElements + index;
    memmove(base + arrayView.mSize, base, sizeof(ElementType) * (this->mSize - index));

    for (uint32 i = 0; i < arrayView.mSize; ++i)
    {
        new (base + i) ElementType(arrayView.mElements[i]);
    }

    this->mSize += arrayView.mSize;
    return IteratorType(this, index);
}

template<typename ElementType>
bool DynArray<ElementType>::Erase(const ConstIteratorType& iterator)
{
    if (iterator == this->End())
    {
        return false;
    }

    this->mElements[iterator.mIndex].~ElementType();

    ElementType* base = this->mElements + iterator.mIndex;
    memmove(base, base + 1, sizeof(ElementType) * (this->mSize - iterator.mIndex - 1));
    this->mSize--;
    return true;
}

template<typename ElementType>
bool DynArray<ElementType>::Erase(const ConstIteratorType& first, const ConstIteratorType& last)
{
    if (first.mIndex >= last.mIndex)
    {
        // nothing to do
        return false;
    }

    // call destructors
    for (int32 i = first.mIndex; i < last.mIndex; ++i)
    {
        this->mElements[i].~ElementType();
    }

    const int32 num = last.mIndex - first.mIndex;
    ElementType* base = this->mElements + first.mIndex;
    memmove(base, base + num, sizeof(ElementType) * (this->mSize - last.mIndex));
    this->mSize -= num;
    return true;
}

template<typename ElementType>
bool DynArray<ElementType>::Reserve(uint32 size)
{
    if (size <= mAllocSize)
    {
        // smaller that allocated - ignore
        return true;
    }

    const uint32 initialSize = 4;
    uint32 newAllocSize = Math::Max(mAllocSize, initialSize);
    while (size > newAllocSize)
    {
        // grow by 50%
        newAllocSize += newAllocSize / 2;
    }

    ElementType* newBuffer = static_cast<ElementType*>(NFE_MALLOC(newAllocSize * sizeof(ElementType), alignof(ElementType)));
    if (!newBuffer)
    {
        // memory allocation failed
        return false;
    }

    // move elements
    for (uint32 i = 0; i < this->mSize; ++i)
    {
        MemoryHelpers::Move<ElementType>(newBuffer + i, this->mElements + i);
    }

    // replace buffer
    NFE_FREE(this->mElements);
    this->mElements = newBuffer;
    mAllocSize = newAllocSize;
    return true;
}

template<typename ElementType>
bool DynArray<ElementType>::Resize(uint32 size)
{
    Clear();
    if (!Reserve(size))
        return false;

    for (uint32 i = 0; i < size; ++i)
    {
        new (this->mElements + i) ElementType;
    }

    this->mSize = size;
    return true;
}

template<typename ElementType>
void DynArray<ElementType>::Swap(DynArray& other)
{
    std::swap(this->mElements, other->mElements);
    std::swap(this->mSize, other->mSize);
    std::swap(mAllocSize, other.mAllocSize);
}


} // namespace Common
} // namespace NFE
