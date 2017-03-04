/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Dynamic array container definitions
 */

#pragma once

#include "DynArray.hpp"
#include "../Assertion.hpp"
#include "../Math/Math.hpp"

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
{
    mAllocSize = 0;
    mElements = nullptr;
    mSize = 0;

    if (!Reserve(other.mSize))
    {
        LOG_ERROR("Failed to reserve memory for DynArray");
        return;
    }

    mSize = other.mSize;
    for (uint32 i = 0; i < other.mSize; ++i)
    {
        new (mElements + i) ElementType(other.mElements[i]);
    }
}

template<typename ElementType>
DynArray<ElementType>::DynArray(DynArray&& other)
{
    // don't free memory if not needed
    Clear(true);

    mElements = other.mElements;
    mSize = other.mSize;
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

    mSize = other.mSize;
    for (uint32 i = 0; i < other.mSize; ++i)
    {
        new (mElements + i) ElementType(other.mElements[i]);
    }

    return *this;
}

template<typename ElementType>
typename DynArray<ElementType>::DynArray& DynArray<ElementType>::operator = (DynArray&& other)
{
    // don't free memory if not needed
    Clear(true);

    mElements = other.mElements;
    mSize = other.mSize;
    mAllocSize = other.mAllocSize;

    other.mElements = nullptr;
    other.mSize = 0;
    other.mAllocSize = 0;

    return *this;
}

template<typename ElementType>
DynArray<ElementType>::DynArray(const std::initializer_list<ElementType>& list)
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
{
    if (!Reserve(count))
    {
        LOG_ERROR("Failed to reserve memory for DynArray");
        return;
    }

    mSize = count;
    for (uint32 i = 0; i < count; ++i)
    {
        new (mElements + i) ElementType(elements[i]);
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
void DynArray<ElementType>::Clear(bool freeMemory)
{
    if (!mElements)
    {
        // already empty
        return;
    }

    // call destructors
    for (uint32 i = 0; i < mSize; ++i)
    {
        mElements[i].~ElementType();
    }
    
    mSize = 0;

    if (freeMemory)
    {
        NFE_FREE(mElements);
        mElements = nullptr;
        mAllocSize = 0;
    }
}

template<typename ElementType>
typename DynArray<ElementType>::Iterator DynArray<ElementType>::PushBack(const ElementType& element)
{
    if (!Reserve(mSize + 1))
    {
        return End();
    }

    new (mElements + mSize) ElementType(element);
    return Iterator(this, mSize++);
}

template<typename ElementType>
typename DynArray<ElementType>::Iterator DynArray<ElementType>::PushBack(ElementType&& element)
{
    if (!Reserve(mSize + 1))
    {
        // memory allocation failed
        return End();
    }

    new (mElements + mSize) ElementType(std::move(element));
    return Iterator(this, mSize++);
}

template<typename ElementType>
bool DynArray<ElementType>::PushBackArray(const ArrayView<ElementType>& arrayView)
{
    if (arrayView.Empty())
    {
        // empty array - do nothing
        return true;
    }

    if (!Reserve(mSize + arrayView.mSize))
    {
        // memory allocation failed
        return false;
    }

    // copy elements
    for (uint32 i = 0; i < arrayView.mSize; ++i)
    {
        new (mElements + mSize + i) ElementType(arrayView.mElements[i]);
    }

    mSize += arrayView.mSize;
    return true;
}

template<typename ElementType>
bool DynArray<ElementType>::PopBack()
{
    if (Empty())
        return false;

    mElements[--mSize].~ElementType();
    return true;
}

template<typename ElementType>
typename DynArray<ElementType>::Iterator DynArray<ElementType>::InsertAt(uint32 index, const ElementType& element)
{
    if (!Reserve(mSize + 1))
    {
        // memory allocation failed
        return End();
    }

    ElementType* base = mElements + index;
    memmove(base + 1, base, sizeof(ElementType) * (mSize - index));
    new (base) ElementType(element);
    mSize++;
    return Iterator(this, index);
}

template<typename ElementType>
typename DynArray<ElementType>::Iterator DynArray<ElementType>::InsertAt(uint32 index, ElementType&& element)
{
    if (!Reserve(mSize + 1))
    {
        // memory allocation failed
        return End();
    }

    ElementType* base = mElements + index;
    memmove(base + 1, base, sizeof(ElementType) * (mSize - index));
    new (base) ElementType(std::move(element));
    mSize++;
    return Iterator(this, index);
}

template<typename ElementType>
typename DynArray<ElementType>::Iterator DynArray<ElementType>::InsertAt(uint32 index, const ElementType& element, uint32 count)
{
    if (count == 0)
    {
        // nothing to DO
        return End();
    }

    if (!Reserve(mSize + count))
    {
        // memory allocation failed
        return End();
    }

    ElementType* base = mElements + index;
    memmove(base + count, base, sizeof(ElementType) * (mSize - index));

    for (uint32 i = 0; i < count; ++i)
    {
        new (base + i) ElementType(element);
    }

    mSize += count;
    return Iterator(this, index);
}

template<typename ElementType>
typename DynArray<ElementType>::Iterator DynArray<ElementType>::InsertArrayAt(uint32 index, const ArrayView<ElementType>& arrayView)
{
    if (arrayView.Empty())
    {
        // trying to insert empty array - do nothing
        return End();
    }

    if (!Reserve(mSize + arrayView.mSize))
    {
        // memory allocation failed
        return End();
    }

    ElementType* base = mElements + index;
    memmove(base + arrayView.mSize, base, sizeof(ElementType) * (mSize - index));

    for (uint32 i = 0; i < arrayView.mSize; ++i)
    {
        new (base + i) ElementType(arrayView.mElements[i]);
    }

    mSize += arrayView.mSize;
    return Iterator(this, index);
}

template<typename ElementType>
bool DynArray<ElementType>::Erase(const ConstIterator& iterator)
{
    if (iterator == End())
    {
        return false;
    }

    mElements[iterator.mIndex].~ElementType();

    ElementType* base = mElements + iterator.mIndex;
    memmove(base, base + 1, sizeof(ElementType) * (mSize - iterator.mIndex - 1));
    mSize--;
    return true;
}

template<typename ElementType>
bool DynArray<ElementType>::Erase(const ConstIterator& first, const ConstIterator& last)
{
    if (first.mIndex >= last.mIndex)
    {
        // nothing to do
        return false;
    }

    // call destructors
    for (uint32 i = first.mIndex; i < last.mIndex; ++i)
    {
        mElements[i].~ElementType();
    }

    const uint32 num = last.mIndex - first.mIndex;
    ElementType* base = mElements + first.mIndex;
    memmove(base, base + num, sizeof(ElementType) * (mSize - last.mIndex));
    mSize -= num;
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

    ElementType* newBuffer = static_cast<ElementType*>(NFE_MALLOC(newAllocSize * sizeof(ElementType), sizeof(ElementType)));
    if (!newBuffer)
    {
        // memory allocation failed
        return false;
    }

    // copy elements
    memcpy(newBuffer, mElements, sizeof(ElementType) * mSize);

    // replace buffer
    NFE_FREE(mElements);
    mElements = newBuffer;
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
        new (mElements + i) ElementType;
    }

    mSize = size;
    return true;
}

template<typename ElementType>
void DynArray<ElementType>::Swap(DynArray& other)
{
    std::swap(mElements, other.mElements);
    std::swap(mSize, other.mSize);
    std::swap(mAllocSize, other.mAllocSize);
}


} // namespace Common
} // namespace NFE
