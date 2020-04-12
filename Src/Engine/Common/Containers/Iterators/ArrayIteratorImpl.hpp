/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Array iterators definitions.
 */

#pragma once

#include "ArrayIterator.hpp"

#include "../../System/Assertion.hpp"

namespace NFE {
namespace Common {


template<typename ElementType>
ConstArrayIterator<ElementType>::ConstArrayIterator(const ArrayIterator<const ElementType>& other)
    : mElements(other.mElements)
    , mIndex(other.mIndex)
{ }

template<typename ElementType>
ConstArrayIterator<ElementType>::ConstArrayIterator(const ConstArrayIterator<const ElementType>& other)
    : mElements(other.mElements)
    , mIndex(other.mIndex)
{ }

template<typename ElementType>
bool ConstArrayIterator<ElementType>::operator == (const ConstArrayIterator& other) const
{
    NFE_ASSERT(this->mElements == other.mElements, "Comparing incompatible iterators");
    return this->mIndex == other.mIndex;
}

template<typename ElementType>
bool ConstArrayIterator<ElementType>::operator != (const ConstArrayIterator& other) const
{
    NFE_ASSERT(this->mElements == other.mElements, "Comparing incompatible iterators");
    return this->mIndex != other.mIndex;
}

template<typename ElementType>
bool ConstArrayIterator<ElementType>::operator < (const ConstArrayIterator& other) const
{
    NFE_ASSERT(this->mElements == other.mElements, "Comparing incompatible iterators");
    return this->mIndex < other.mIndex;
}

template<typename ElementType>
ptrdiff_t ConstArrayIterator<ElementType>::operator-(const ConstArrayIterator& rhs) const
{
    NFE_ASSERT(this->mElements == rhs.mElements, "Comparing incompatible iterators");
    return static_cast<ptrdiff_t>(this->mIndex) - static_cast<ptrdiff_t>(rhs.mIndex);
}

template<typename ElementType>
const ElementType& ConstArrayIterator<ElementType>::operator*() const
{
    return this->mElements[this->mIndex];
}

template<typename ElementType>
const ElementType* ConstArrayIterator<ElementType>::operator->() const
{
    return this->mElements + this->mIndex;
}

template<typename ElementType>
const ElementType& ConstArrayIterator<ElementType>::operator [] (ptrdiff_t index) const
{
    return this->mElements[this->mIndex + index];
}

template<typename ElementType>
ConstArrayIterator<ElementType>& ConstArrayIterator<ElementType>::operator++()
{
    this->mIndex++;
    return *this;
}

template<typename ElementType>
ConstArrayIterator<ElementType> ConstArrayIterator<ElementType>::operator++(int)
{
    return ConstArrayIterator(this->mElements, this->mIndex++);
}

template<typename ElementType>
ConstArrayIterator<ElementType>& ConstArrayIterator<ElementType>::operator+=(ptrdiff_t offset)
{
    this->mIndex += static_cast<int32>(offset);
    return *this;
}

template<typename ElementType>
ConstArrayIterator<ElementType>& ConstArrayIterator<ElementType>::operator--()
{
    NFE_ASSERT(this->mIndex > 0, "Iterating beyond array range");
    this->mIndex--;
    return *this;
}

template<typename ElementType>
ConstArrayIterator<ElementType> ConstArrayIterator<ElementType>::operator--(int)
{
    NFE_ASSERT(this->mIndex > 0, "Iterating beyond array range");
    return ConstArrayIterator(this->mElements, this->mIndex--);
}

template<typename ElementType>
ConstArrayIterator<ElementType>& ConstArrayIterator<ElementType>::operator-=(ptrdiff_t offset)
{
    this->mIndex -= static_cast<int32>(offset);
    return *this;
}

template<typename ElementType>
ConstArrayIterator<ElementType> ConstArrayIterator<ElementType>::operator+(ptrdiff_t offset) const
{
    return ConstArrayIterator(this->mElements, this->mIndex + static_cast<int32>(offset));
}

template<typename ElementType>
ConstArrayIterator<ElementType> ConstArrayIterator<ElementType>::operator-(ptrdiff_t offset) const
{
    return ConstArrayIterator(this->mElements, this->mIndex - static_cast<int32>(offset));
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
ptrdiff_t ArrayIterator<ElementType>::operator-(const ArrayIterator& rhs) const
{
    return ConstArrayIterator<ElementType>::operator - (rhs);
}

template<typename ElementType>
ElementType& ArrayIterator<ElementType>::operator*() const
{
    return GetElements()[this->mIndex];
}

template<typename ElementType>
ElementType* ArrayIterator<ElementType>::operator->() const
{
    return GetElements() + this->mIndex;
}

template<typename ElementType>
ElementType& ArrayIterator<ElementType>::operator [] (ptrdiff_t index) const
{
    return GetElements()[this->mIndex + index];
}

template<typename ElementType>
ArrayIterator<ElementType>& ArrayIterator<ElementType>::operator++()
{
    this->mIndex++;
    return *this;
}

template<typename ElementType>
ArrayIterator<ElementType> ArrayIterator<ElementType>::operator++(int)
{
    return ArrayIterator(GetElements(), this->mIndex++);
}

template<typename ElementType>
ArrayIterator<ElementType>& ArrayIterator<ElementType>::operator+=(ptrdiff_t offset)
{
    this->mIndex += static_cast<int32>(offset);
    return *this;
}

template<typename ElementType>
ArrayIterator<ElementType>& ArrayIterator<ElementType>::operator--()
{
    NFE_ASSERT(this->mIndex > 0, "Iterating beyond array range");
    this->mIndex--;
    return *this;
}

template<typename ElementType>
ArrayIterator<ElementType> ArrayIterator<ElementType>::operator--(int)
{
    NFE_ASSERT(this->mIndex > 0, "Iterating beyond array range");
    return ArrayIterator(GetElements(), this->mIndex--);
}

template<typename ElementType>
ArrayIterator<ElementType>& ArrayIterator<ElementType>::operator-=(ptrdiff_t offset)
{
    this->mIndex -= static_cast<int32>(offset);
    return *this;
}

template<typename ElementType>
ArrayIterator<ElementType> ArrayIterator<ElementType>::operator+(ptrdiff_t offset) const
{
    return ArrayIterator(GetElements(), this->mIndex + static_cast<int32>(offset));
}

template<typename ElementType>
ArrayIterator<ElementType> ArrayIterator<ElementType>::operator-(ptrdiff_t offset) const
{
    return ArrayIterator(GetElements(), this->mIndex - static_cast<int32>(offset));
}


} // namespace Common
} // namespace NFE
