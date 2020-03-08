/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Static array container definitions
 */

#pragma once

#include "StaticArray.hpp"
#include "../System/Assertion.hpp"
#include "../Memory/MemoryHelpers.hpp"


namespace NFE {
namespace Common {

template<typename ElementType, uint32 MaxSize>
StaticArray<ElementType, MaxSize>::StaticArray()
    : mSize(0)
{
}

template<typename ElementType, uint32 MaxSize>
StaticArray<ElementType, MaxSize>::~StaticArray()
{
    Clear();
}

template<typename ElementType, uint32 MaxSize>
StaticArray<ElementType, MaxSize>::StaticArray(const StaticArray& other)
    : mSize(other.mSize)
{
    for (uint32 i = 0; i < other.mSize; ++i)
    {
        new (Data() + i) ElementType(other[i]);
    }
}

template<typename ElementType, uint32 MaxSize>
StaticArray<ElementType, MaxSize>::StaticArray(StaticArray&& other)
    : mSize(other.mSize)
{
    for (uint32 i = 0; i < this->mSize; ++i)
    {
        new (Data() + i) ElementType(std::move(other.Data()[i]));
        other.Data()[i].~ElementType();
    }

    other.mSize = 0;
}

template<typename ElementType, uint32 MaxSize>
StaticArray<ElementType, MaxSize>& StaticArray<ElementType, MaxSize>::operator = (const StaticArray& other)
{
    if (&other == this)
        return *this;

    Clear();

    this->mSize = other.mSize;

    for (uint32 i = 0; i < other.mSize; ++i)
    {
        new (Data() + i) ElementType(other[i]);
    }

    return *this;
}

template<typename ElementType, uint32 MaxSize>
StaticArray<ElementType, MaxSize>& StaticArray<ElementType, MaxSize>::operator = (StaticArray&& other)
{
    if (&other == this)
        return *this;

    Clear();

    this->mSize = other.mSize;
    other.mSize = 0;

    for (uint32 i = 0; i < this->mSize; ++i)
    {
        Data()[i] = std::move(other.Data()[i]);
        other.Data()[i].~ElementType();
    }

    return *this;
}

template<typename ElementType, uint32 MaxSize>
StaticArray<ElementType, MaxSize>::StaticArray(const std::initializer_list<ElementType>& list)
    : StaticArray()
{
    NFE_ASSERT(static_cast<uint32>(list.size()) <= MaxSize, "Initializer list is too big to fit the static array");

    for (const ElementType element : list)
    {
        PushBack(element);
    }
}

template<typename ElementType, uint32 MaxSize>
StaticArray<ElementType, MaxSize>::StaticArray(const ElementType* elements, uint32 count)
    : StaticArray()
{
    NFE_ASSERT(count <= MaxSize, "Too many elements");

    this->mSize = count;
    for (uint32 i = 0; i < count; ++i)
    {
        new (Data() + i) ElementType(elements[i]);
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename ElementType, uint32 MaxSize>
ArrayView<ElementType> StaticArray<ElementType, MaxSize>::GetView()
{
    return ArrayView<ElementType>(Data(), Size());
}

template<typename ElementType, uint32 MaxSize>
ArrayView<const ElementType> StaticArray<ElementType, MaxSize>::GetView() const
{
    return ArrayView<const ElementType>(Data(), Size());
}

template<typename ElementType, uint32 MaxSize>
uint32 StaticArray<ElementType, MaxSize>::Size() const
{
    return mSize;
}

template<typename ElementType, uint32 MaxSize>
bool StaticArray<ElementType, MaxSize>::Empty() const
{
    return mSize == 0;
}

template<typename ElementType, uint32 MaxSize>
const ElementType* StaticArray<ElementType, MaxSize>::Data() const
{
    return reinterpret_cast<const ElementType*>(mData);
}

template<typename ElementType, uint32 MaxSize>
ElementType* StaticArray<ElementType, MaxSize>::Data()
{
    return reinterpret_cast<ElementType*>(mData);
}

template<typename ElementType, uint32 MaxSize>
const ElementType& StaticArray<ElementType, MaxSize>::Front() const
{
    NFE_ASSERT(mSize > 0, "Array is empty");
    return Data()[0];
}

template<typename ElementType, uint32 MaxSize>
ElementType& StaticArray<ElementType, MaxSize>::Front()
{
    NFE_ASSERT(mSize > 0, "Array is empty");
    return  Data()[0];
}

template<typename ElementType, uint32 MaxSize>
const ElementType& StaticArray<ElementType, MaxSize>::Back() const
{
    NFE_ASSERT(mSize > 0, "Array is empty");
    return  Data()[mSize - 1];
}

template<typename ElementType, uint32 MaxSize>
ElementType& StaticArray<ElementType, MaxSize>::Back()
{
    static_assert(!std::is_const<ElementType>::value, "You can only use ConstIterator for const-typed ArrayView");
    NFE_ASSERT(mSize > 0, "Array is empty");
    return  Data()[mSize - 1];
}

template<typename ElementType, uint32 MaxSize>
typename StaticArray<ElementType, MaxSize>::ConstIterator StaticArray<ElementType, MaxSize>::Begin() const
{
    const ArrayView<const ElementType> view = GetView();
    return view.Begin();
}

template<typename ElementType, uint32 MaxSize>
typename StaticArray<ElementType, MaxSize>::Iterator StaticArray<ElementType, MaxSize>::Begin()
{
    ArrayView<ElementType> view = GetView();
    return view.Begin();
}

template<typename ElementType, uint32 MaxSize>
typename StaticArray<ElementType, MaxSize>::ConstIterator StaticArray<ElementType, MaxSize>::End() const
{
    const ArrayView<const ElementType> view = GetView();
    return view.End();
}

template<typename ElementType, uint32 MaxSize>
typename StaticArray<ElementType, MaxSize>::Iterator StaticArray<ElementType, MaxSize>::End()
{
    ArrayView<ElementType> view = GetView();
    return view.End();
}

template<typename ElementType, uint32 MaxSize>
ElementType& StaticArray<ElementType, MaxSize>::operator [] (uint32 index)
{
    return Data()[index];
}

template<typename ElementType, uint32 MaxSize>
const ElementType& StaticArray<ElementType, MaxSize>::operator [] (uint32 index) const
{
    return Data()[index];
}

template<typename ElementType, uint32 MaxSize>
void StaticArray<ElementType, MaxSize>::Clear()
{
    // call destructors
    for (uint32 i = 0; i < this->mSize; ++i)
    {
        Data()[i].~ElementType();
    }

    this->mSize = 0;
}

template<typename ElementType, uint32 MaxSize>
typename StaticArray<ElementType, MaxSize>::Iterator StaticArray<ElementType, MaxSize>::PushBack(const ElementType& element)
{
    NFE_ASSERT(mSize < MaxSize, "StaticArray size exceeded");

    new (Data() + this->mSize) ElementType(element);

    const uint32 index = this->mSize++;
    return GetView().Begin() + index;
}

template<typename ElementType, uint32 MaxSize>
typename StaticArray<ElementType, MaxSize>::Iterator StaticArray<ElementType, MaxSize>::PushBack(ElementType&& element)
{
    NFE_ASSERT(mSize < MaxSize, "StaticArray size exceeded");

    new (Data() + this->mSize) ElementType(std::move(element));

    const uint32 index = this->mSize++;
    return GetView().Begin() + index;
}

template<typename ElementType, uint32 MaxSize>
template<typename ElementType2>
bool StaticArray<ElementType, MaxSize>::PushBackArray(const ArrayView<ElementType2>& arrayView)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, typename std::remove_cv<ElementType2>::type>::value,
                  "Incompatible element types");

    if (arrayView.Empty())
    {
        // empty array - do nothing
        return true;
    }

    NFE_ASSERT(mSize + arrayView.Size() <= MaxSize, "StaticArray size exceeded");

    // copy elements
    for (uint32 i = 0; i < arrayView.Size(); ++i)
    {
        new (Data() + this->mSize + i) ElementType(arrayView[i]);
    }

    this->mSize += arrayView.Size();
    return true;
}

template<typename ElementType, uint32 MaxSize>
bool StaticArray<ElementType, MaxSize>::PopBack()
{
    if (this->Empty())
        return false;

    Data()[--this->mSize].~ElementType();
    return true;
}

template<typename ElementType, uint32 MaxSize>
typename StaticArray<ElementType, MaxSize>::Iterator StaticArray<ElementType, MaxSize>::InsertAt(uint32 index, const ElementType& element)
{
    NFE_ASSERT(mSize < MaxSize, "StaticArray size exceeded");

    ElementType* base = Data() + index;
    MemoryHelpers::MoveArray<ElementType>(base + 1, base, this->mSize - index);
    new (base) ElementType(element);
    this->mSize++;

    return GetView().Begin() + index;
}

template<typename ElementType, uint32 MaxSize>
typename StaticArray<ElementType, MaxSize>::Iterator StaticArray<ElementType, MaxSize>::InsertAt(uint32 index, ElementType&& element)
{
    NFE_ASSERT(mSize < MaxSize, "StaticArray size exceeded");

    ElementType* base = Data() + index;
    MemoryHelpers::MoveArray<ElementType>(base + 1, base, this->mSize - index);
    new (base) ElementType(std::move(element));
    this->mSize++;

    return GetView().Begin() + index;
}

template<typename ElementType, uint32 MaxSize>
typename StaticArray<ElementType, MaxSize>::Iterator StaticArray<ElementType, MaxSize>::InsertAt(uint32 index, const ElementType& element, uint32 count)
{
    if (count == 0)
    {
        // nothing to do
        return this->End();
    }

    NFE_ASSERT(mSize + count <= MaxSize, "StaticArray size exceeded");

    ElementType* base = Data() + index;
    MemoryHelpers::MoveArray<ElementType>(base + count, base, this->mSize - index);

    for (uint32 i = 0; i < count; ++i)
    {
        new (base + i) ElementType(element);
    }

    this->mSize += count;

    return GetView().Begin() + index;
}

template<typename ElementType, uint32 MaxSize>
template<typename ElementType2>
typename StaticArray<ElementType, MaxSize>::Iterator StaticArray<ElementType, MaxSize>::InsertArrayAt(uint32 index, const ArrayView<ElementType2>& arrayView)
{
    static_assert(std::is_same<typename std::remove_cv<ElementType>::type, typename std::remove_cv<ElementType2>::type>::value,
                  "Incompatible element types");

    if (arrayView.Empty())
    {
        // trying to insert empty array - do nothing
        return this->End();
    }

    NFE_ASSERT(mSize + arrayView.Size() <= MaxSize, "StaticArray size exceeded");

    ElementType* base = Data() + index;
    MemoryHelpers::MoveArray<ElementType>(base + arrayView.Size(), base, this->mSize - index);

    for (uint32 i = 0; i < arrayView.Size(); ++i)
    {
        new (base + i) ElementType(arrayView[i]);
    }

    this->mSize += arrayView.Size();

    return GetView().Begin() + index;
}

template<typename ElementType, uint32 MaxSize>
bool StaticArray<ElementType, MaxSize>::Erase(const ConstIterator& iterator)
{
    if (iterator == this->End())
    {
        return false;
    }

    const int32 index = iterator.GetIndex();
    Data()[index].~ElementType();

    ElementType* base = Data() + index;
    MemoryHelpers::MoveArray<ElementType>(base, base + 1, this->mSize - index - 1);
    this->mSize--;
    return true;
}

template<typename ElementType, uint32 MaxSize>
bool StaticArray<ElementType, MaxSize>::Erase(const ConstIterator& first, const ConstIterator& last)
{
    if (first.GetIndex() >= last.GetIndex())
    {
        // nothing to do
        return false;
    }

    // call destructors
    for (int32 i = first.GetIndex(); i < last.GetIndex(); ++i)
    {
        Data()[i].~ElementType();
    }

    const int32 num = last.GetIndex() - first.GetIndex();
    ElementType* base = Data() + first.GetIndex();
    MemoryHelpers::MoveArray<ElementType>(base, base + num, this->mSize - last.GetIndex());
    this->mSize -= num;
    return true;
}

template<typename ElementType, uint32 MaxSize>
bool StaticArray<ElementType, MaxSize>::Resize(uint32 size)
{
    NFE_ASSERT(size <= MaxSize, "StaticArray size exceeded");

    for (uint32 i = 0; i < size; ++i)
    {
        new (Data() + i) ElementType;
    }

    this->mSize = size;
    return true;
}


} // namespace Common
} // namespace NFE
