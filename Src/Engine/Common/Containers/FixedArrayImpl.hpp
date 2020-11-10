/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Fixed array container definitions
 */

#pragma once

#include "FixedArray.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Common {

template<typename ElementType, uint32 ArraySize>
FixedArray<ElementType, ArraySize>::FixedArray(const ElementType& defaultElement)
{
    for (uint32 i = 0; i < ArraySize; ++i)
    {
        new (mElements + i) ElementType(defaultElement);
    }
}

template<typename ElementType, uint32 ArraySize>
FixedArray<ElementType, ArraySize>::FixedArray(const FixedArray& other)
{
    for (uint32 i = 0; i < ArraySize; ++i)
    {
        new (mElements + i) ElementType(other.mElements[i]);
    }
}

template<typename ElementType, uint32 ArraySize>
FixedArray<ElementType, ArraySize>::FixedArray(FixedArray&& other)
{
    for (uint32 i = 0; i < ArraySize; ++i)
    {
        new (mElements + i) ElementType(std::move(other.mElements[i]));
    }
}

template<typename ElementType, uint32 ArraySize>
FixedArray<ElementType, ArraySize>& FixedArray<ElementType, ArraySize>::operator = (const FixedArray& other)
{
    if (&other != this)
    {
        for (uint32 i = 0; i < ArraySize; ++i)
        {
            mElements[i] = other.mElements[i];
        }
    }

    return *this;
}

template<typename ElementType, uint32 ArraySize>
FixedArray<ElementType, ArraySize>& FixedArray<ElementType, ArraySize>::operator = (FixedArray&& other)
{
    if (&other != this)
    {
        for (uint32 i = 0; i < ArraySize; ++i)
        {
            mElements[i] = std::move(other.mElements[i]);
        }
    }

    return *this;
}

template<typename ElementType, uint32 ArraySize>
FixedArray<ElementType, ArraySize>::FixedArray(const std::initializer_list<ElementType>& list)
{
    NFE_ASSERT(static_cast<uint32>(list.size()) == ArraySize, "Initializer list size must match fixed array size");

    for (uint32 i = 0; i < ArraySize; ++i)
    {
        new (mElements + i) ElementType(list.begin()[i]);
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType, uint32 ArraySize>
ArrayView<ElementType> FixedArray<ElementType, ArraySize>::GetView()
{
    return ArrayView<ElementType>(Data(), ArraySize);
}

template<typename ElementType, uint32 ArraySize>
ArrayView<const ElementType> FixedArray<ElementType, ArraySize>::GetView() const
{
    return ArrayView<const ElementType>(Data(), ArraySize);
}

template<typename ElementType, uint32 ArraySize>
const ElementType& FixedArray<ElementType, ArraySize>::Front() const
{
    return mElements[0];
}

template<typename ElementType, uint32 ArraySize>
ElementType& FixedArray<ElementType, ArraySize>::Front()
{
    return mElements[0];
}

template<typename ElementType, uint32 ArraySize>
const ElementType& FixedArray<ElementType, ArraySize>::Back() const
{
    return mElements[ArraySize - 1];
}

template<typename ElementType, uint32 ArraySize>
ElementType& FixedArray<ElementType, ArraySize>::Back()
{
    static_assert(!std::is_const<ElementType>::value, "You can only use ConstIterator for const-typed FixedArray");
    return mElements[ArraySize - 1];
}

template<typename ElementType, uint32 ArraySize>
typename FixedArray<ElementType, ArraySize>::ConstIterator FixedArray<ElementType, ArraySize>::Begin() const
{
    const ArrayView<const ElementType> view = GetView();
    return view.Begin();
}

template<typename ElementType, uint32 ArraySize>
typename FixedArray<ElementType, ArraySize>::Iterator FixedArray<ElementType, ArraySize>::Begin()
{
    ArrayView<ElementType> view = GetView();
    return view.Begin();
}

template<typename ElementType, uint32 ArraySize>
typename FixedArray<ElementType, ArraySize>::ConstIterator FixedArray<ElementType, ArraySize>::End() const
{
    const ArrayView<const ElementType> view = GetView();
    return view.End();
}

template<typename ElementType, uint32 ArraySize>
typename FixedArray<ElementType, ArraySize>::Iterator FixedArray<ElementType, ArraySize>::End()
{
    ArrayView<ElementType> view = GetView();
    return view.End();
}

template<typename ElementType, uint32 ArraySize>
ElementType& FixedArray<ElementType, ArraySize>::operator [] (uint32 index)
{
    NFE_ASSERT(index < ArraySize, "Fixed array index out of bounds. Index is %u, while array has only %u elements", index, ArraySize);
    return mElements[index];
}

template<typename ElementType, uint32 ArraySize>
const ElementType& FixedArray<ElementType, ArraySize>::operator [] (uint32 index) const
{
    NFE_ASSERT(index < ArraySize, "Fixed array index out of bounds. Index is %u, while array has only %u elements", index, ArraySize);
    return mElements[index];
}


} // namespace Common
} // namespace NFE
