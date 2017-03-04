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
    // TODO
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
    // TODO
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
        return End();
    }

    new (mElements + mSize) ElementType(std::move(element));
    return Iterator(this, mSize++);
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
    // TODO
    UNUSED(index);
    UNUSED(element);
    return End();
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
bool DynArray<ElementType>::Erase(const ConstIterator& iterator)
{
    // TODO
    UNUSED(iterator);
    return End();
}

template<typename ElementType>
bool DynArray<ElementType>::Erase(const Iterator& iterator)
{
    // TODO
    UNUSED(iterator);
    return End();
}

template<typename ElementType>
bool DynArray<ElementType>::Reserve(uint32 size)
{
    if (size <= mAllocSize)
    {
        // smaller that allocated - ignore
        return true;
    }

    // TODO perform relocation
    return false;
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
