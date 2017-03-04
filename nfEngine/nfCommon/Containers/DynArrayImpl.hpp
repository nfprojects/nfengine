/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Dynamic array container definitions
 */

#pragma once

#include "DynArray.hpp"
#include "../Assertion.hpp"


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
    if (mElements)
    {
        // TODO call destructors
        NFE_FREE(mElements);
        mElements = nullptr;
        mSize = 0;
        mAllocSize = 0;
    }
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
    if (mElements)
    {
        // TODO call destructors
        NFE_FREE(mElements);
    }

    mElements = other.mElements;
    mSize = other.mSize;
    mAllocSize = other.mAllocSize;

    other.mElements = nullptr;
    other.mSize = 0;
    other.mAllocSize = 0;
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
void DynArray<ElementType>::Clear(bool freeMemory)
{
    if (freeMemory)
    {
        // TODO
    }

    mAllocSize = 0;
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
bool DynArray<ElementType>::PushBack(const ArrayView<ElementType>& arrayView)
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

    memmove(mElements + (index + 1), mElements + index, sizeof(ElementType) * )


    // TODO
    UNUSED(index);
    UNUSED(element);
    return Iterator(this, index);
}

template<typename ElementType>
typename DynArray<ElementType>::Iterator DynArray<ElementType>::InsertAt(uint32 index, ElementType&& element)
{
    // TODO
    UNUSED(index);
    UNUSED(element);
    return End();
}

template<typename ElementType>
typename DynArray<ElementType>::Iterator DynArray<ElementType>::InsertAt(uint32 index, const ArrayView<ElementType>& arrayView)
{
    // TODO
    UNUSED(index);
    UNUSED(arrayView);
    return End();
}

template<typename ElementType>
bool DynArray<ElementType>::Erase(const ConstIterator& iterator)
{
    // TODO
    UNUSED(iterator);
    return false;
}

template<typename ElementType>
bool DynArray<ElementType>::Erase(const Iterator& iterator)
{
    // TODO
    UNUSED(iterator);
    return false;
}

template<typename ElementType>
bool DynArray<ElementType>::Erase(const ConstIterator& first, const ConstIterator& last)
{
    // TODO
    UNUSED(first);
    UNUSED(last);
    return false;
}

template<typename ElementType>
bool DynArray<ElementType>::Erase(const Iterator& first, const Iterator& last)
{
    // TODO
    UNUSED(first);
    UNUSED(last);
    return false;
}

template<typename ElementType>
bool DynArray<ElementType>::Reserve(uint32 size)
{
    if (size <= mAllocSize)
    {
        // smaller that allocated - ignore
        return true;
    }

    uint32 newAllocSize = mAllocSize;
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
    // TODO
    UNUSED(size);
    return false;
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
