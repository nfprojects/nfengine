/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Dynamic array container definitions
 */

#pragma once

#include "ArrayView.hpp"
#include "../Assertion.hpp"


namespace NFE {
namespace Common {

template<typename ElementType>
ArrayView<ElementType>::ArrayView()
    : mElements(nullptr)
    , mSize(0)
{
}

template<typename ElementType>
ArrayView<ElementType>::ArrayView(const ArrayView& other)
{
    mElements = other.mElements;
    mSize = mSize;
}

template<typename ElementType>
typename ArrayView<ElementType>::ArrayView& ArrayView<ElementType>::operator = (const ArrayView& other)
{
    mElements = other.mElements;
    mSize = other.mSize;
    return *this;
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
uint32 ArrayView<ElementType>::Size() const
{
    return mSize;
}

template<typename ElementType>
uint32 ArrayView<ElementType>::MaxSize()
{
    return (UINT32_MAX - 1) / static_cast<uint32>(sizeof(ElementType));
}

template<typename ElementType>
bool ArrayView<ElementType>::Empty() const
{
    return mSize > 0;
}

template<typename ElementType>
const ElementType& ArrayView<ElementType>::Front() const
{
    NFE_ASSERT(mSize > 0, "Array is empty");
    return mElements[0];
}

template<typename ElementType>
ElementType& ArrayView<ElementType>::Front()
{
    NFE_ASSERT(mSize > 0, "Array is empty");
    return mElements[0];
}

template<typename ElementType>
const ElementType& ArrayView<ElementType>::Back() const
{
    NFE_ASSERT(mSize > 0, "Array is empty");
    return mElements[mSize - 1];
}

template<typename ElementType>
ElementType& ArrayView<ElementType>::Back()
{
    NFE_ASSERT(mSize > 0, "Array is empty");
    return mElements[mSize - 1];
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::Begin() const
{
    return ConstIterator(this, 0);
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::Begin()
{
    return Iterator(this, 0);
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::End() const
{
    return ConstIterator(this, mSize);
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::End()
{
    return Iterator(this, mSize);
}

template<typename ElementType>
ElementType& ArrayView<ElementType>::operator[](uint32 index)
{
    NFE_ASSERT(index < mSize, "Invalid array index %u (size is %u)", index, mSize);
    return mElements[index];
}

template<typename ElementType>
const ElementType& ArrayView<ElementType>::operator[](uint32 index) const
{
    NFE_ASSERT(index < mSize, "Invalid array index %u (size is %u)", index, mSize);
    return mElements[index];
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::Find(const ElementType& element) const
{
    for (uint32 i = 0; i < mSize; ++i)
    {
        if (mElements[i] == element)
        {
            return ConstIterator(this, i);
        }
    }

    return End();
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::Find(const ElementType& element)
{
    for (uint32 i = 0; i < mSize; ++i)
    {
        if (mElements[i] == element)
        {
            return Iterator(this, i);
        }
    }

    return End();
}


} // namespace Common
} // namespace NFE
