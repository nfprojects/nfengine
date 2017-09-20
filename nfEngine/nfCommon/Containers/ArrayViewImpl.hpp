/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Dynamic array container definitions
 */

#pragma once

#include "ArrayView.hpp"
#include "../System/Assertion.hpp"


namespace NFE {
namespace Common {


template<typename ElementType>
bool ArrayView<ElementType>::ConstIterator::operator == (const ConstIterator& other) const
{
    NFE_ASSERT(mArray == other.mArray, "Comparing incompatible iterators");
    return mIndex == other.mIndex;
}

template<typename ElementType>
bool ArrayView<ElementType>::ConstIterator::operator != (const ConstIterator& other) const
{
    NFE_ASSERT(mArray == other.mArray, "Comparing incompatible iterators");
    return mIndex != other.mIndex;
}

template<typename ElementType>
bool ArrayView<ElementType>::ConstIterator::operator < (const ConstIterator& other) const
{
    NFE_ASSERT(mArray == other.mArray, "Comparing incompatible iterators");
    return mIndex < other.mIndex;
}

template<typename ElementType>
int32 ArrayView<ElementType>::ConstIterator::operator-(const ConstIterator& rhs) const
{
    NFE_ASSERT(this->mArray == rhs.mArray, "Comparing incompatible iterators");
    return static_cast<int32>(this->mIndex) - static_cast<int32>(rhs.mIndex);
}

template<typename ElementType>
const ElementType& ArrayView<ElementType>::ConstIterator::operator*() const
{
    return mArray->mElements[mIndex];
}

template<typename ElementType>
const ElementType* ArrayView<ElementType>::ConstIterator::operator->() const
{
    return mArray->mElements + mIndex;
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator& ArrayView<ElementType>::ConstIterator::operator++()
{
    mIndex++;
    return *this;
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::ConstIterator::operator++(int)
{
    return ConstIterator(mArray, mIndex++);
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator& ArrayView<ElementType>::ConstIterator::operator+=(int32 offset)
{
    mIndex += offset;
    return *this;
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator& ArrayView<ElementType>::ConstIterator::operator--()
{
    mIndex--;
    return *this;
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::ConstIterator::operator--(int)
{
    return ConstIterator(mArray, mIndex--);
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator& ArrayView<ElementType>::ConstIterator::operator-=(int32 offset)
{
    mIndex -= offset;
    return *this;
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::ConstIterator::operator+(int32 offset) const
{
    return ConstIterator(mArray, mIndex + offset);
}

template<typename ElementType>
typename ArrayView<ElementType>::ConstIterator ArrayView<ElementType>::ConstIterator::operator-(int32 offset) const
{
    return ConstIterator(mArray, mIndex - offset);
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
int32 ArrayView<ElementType>::Iterator::operator-(const Iterator& rhs) const
{
    return ConstIterator::operator - (rhs);
}

template<typename ElementType>
ElementType& ArrayView<ElementType>::Iterator::operator*() const
{
    // 'this' is required, because compiler does not now that 'mArray' is dependent type
    return this->mArray->mElements[this->mIndex];
}

template<typename ElementType>
ElementType* ArrayView<ElementType>::Iterator::operator->() const
{
    return this->mArray->mElements + this->mIndex;
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator& ArrayView<ElementType>::Iterator::operator++()
{
    this->mIndex++;
    return *this;
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::Iterator::operator++(int)
{
    return Iterator(this->mArray, this->mIndex++);
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator& ArrayView<ElementType>::Iterator::operator+=(int32 offset)
{
    this->mIndex += offset;
    return *this;
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator& ArrayView<ElementType>::Iterator::operator--()
{
    this->mIndex--;
    return *this;
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::Iterator::operator--(int)
{
    return Iterator(this->mArray, this->mIndex--);
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator& ArrayView<ElementType>::Iterator::operator-=(int32 offset)
{
    this->mIndex -= offset;
    return *this;
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::Iterator::operator+(int32 offset) const
{
    return Iterator(this->mArray, this->mIndex + offset);
}

template<typename ElementType>
typename ArrayView<ElementType>::Iterator ArrayView<ElementType>::Iterator::operator-(int32 offset) const
{
    return Iterator(this->mArray, this->mIndex - offset);
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
ArrayView<ElementType>::ArrayView()
    : mElements(nullptr)
    , mSize(0)
{
}

template<typename ElementType>
ArrayView<ElementType>::ArrayView(ElementType* elements, uint32 numElements)
    : mElements(elements)
    , mSize(numElements)
{
}

template<typename ElementType>
template<typename ElementType2>
ArrayView<ElementType>::ArrayView(const ArrayView<ElementType2>& other)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, ElementType2>::value,
                  "Only (non-const -> const) ArrayView element type conversion is supported");

    mElements = other.Data();
    mSize = other.Size();
}

template<typename ElementType>
template<typename ElementType2>
ArrayView<ElementType>& ArrayView<ElementType>::operator = (const ArrayView<ElementType2>& other)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, ElementType2>::value,
                  "Only (non-const -> const) ArrayView element type conversion is supported");

    mElements = other.Data();
    mSize = other.Size();
    return *this;
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType>
uint32 ArrayView<ElementType>::Size() const
{
    return mSize;
}

template<typename ElementType>
bool ArrayView<ElementType>::Empty() const
{
    return mSize == 0;
}

template<typename ElementType>
const ElementType* ArrayView<ElementType>::Data() const
{
    return mElements;
}

template<typename ElementType>
ElementType* ArrayView<ElementType>::Data()
{
    return mElements;
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
    static_assert(!std::is_const<ElementType>::value, "You can only use ConstIterator for const-typed ArrayView");
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
    static_assert(!std::is_const<ElementType>::value, "You can only use ConstIterator for const-typed ArrayView");
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
    static_assert(!std::is_const<ElementType>::value, "You can only use ConstIterator for const-typed ArrayView");
    return Iterator(this, mSize);
}

template<typename ElementType>
ElementType& ArrayView<ElementType>::operator[](uint32 index)
{
    static_assert(!std::is_const<ElementType>::value, "You can only use const-reference to access const-typed ArrayView elements");
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
typename ArrayView<ElementType>::ArrayView ArrayView<ElementType>::Range(uint32 index, uint32 size) const
{
    NFE_ASSERT(index < mSize, "Invalid array index %u (size is %u)", index, mSize);
    NFE_ASSERT(index + size < mSize + 1, "Subrange exceedes array size (last index is %u, size is %u)", index + size, mSize);
    return ArrayView(mElements + index, size);
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

template<typename ElementTypeA, typename ElementTypeB>
bool operator == (const ArrayView<ElementTypeA>& a, const ArrayView<ElementTypeB>& b)
{
    static_assert(std::is_same<typename std::remove_cv<ElementTypeA>::type, typename std::remove_cv<ElementTypeB>::type>::value,
                  "ArrayView types are incompatible");

    if (a.Size() != b.Size())
        return false;

    for (uint32 i = 0; i < a.Size(); ++i)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }

    return true;
}

template<typename ElementTypeA, typename ElementTypeB>
bool operator != (const ArrayView<ElementTypeA>& a, const ArrayView<ElementTypeB>& b)
{
    static_assert(std::is_same<typename std::remove_cv<ElementTypeA>::type, typename std::remove_cv<ElementTypeB>::type>::value,
                  "ArrayView types are incompatible");

    if (a.Size() != b.Size())
        return true;

    for (uint32 i = 0; i < a.Size(); ++i)
    {
        if (a[i] != b[i])
        {
            return true;
        }
    }

    return false;
}


} // namespace Common
} // namespace NFE
