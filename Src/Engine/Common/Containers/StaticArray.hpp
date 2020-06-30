/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Static array declarations
 */

#pragma once

#include "../nfCommon.hpp"
#include "ArrayView.hpp"

#include <iterator>


namespace NFE {
namespace Common {

/**
 * Static array - an array that is not using dynamic allocations (maximum size is fixed).
 * Exceeding maximum capacity will cause asserion.
 */
template<typename ElementType, uint32 MaxSize>
class StaticArray final
{
public:
    static_assert(MaxSize > 0, "Static array's maximum size must be greater than zero");

    using Iterator = typename ArrayView<ElementType>::Iterator;
    using ConstIterator = typename ArrayView<ElementType>::ConstIterator;

    // basic constructors and assignment operators
    NFE_INLINE StaticArray();
    NFE_INLINE ~StaticArray();

    // TODO support 'other' of different size
    NFE_INLINE StaticArray(const StaticArray& other);
    NFE_INLINE StaticArray(StaticArray&& other);
    NFE_INLINE StaticArray& operator = (const StaticArray& other);
    NFE_INLINE StaticArray& operator = (StaticArray&& other);

    // initialize using initializer list
    StaticArray(const std::initializer_list<ElementType>& list);

    // initialize using C-style array
    StaticArray(const ElementType* elements, uint32 count);

    /**
     * Get a view of this array.
     */
    NFE_INLINE ArrayView<ElementType> GetView();
    NFE_INLINE ArrayView<const ElementType> GetView() const;

    /**
     * Get number of elements.
     */
    NFE_INLINE uint32 Size() const;

    /**
     * Get raw data pointed by the view.
     */
    NFE_INLINE const ElementType* Data() const;
    NFE_INLINE ElementType* Data();

    /**
     * Check if the array is empty.
     */
    NFE_INLINE bool Empty() const;

    /**
     * Get first element.
     * @note Array must not be empty. Otherwise it will cause an assertion.
     */
    NFE_INLINE const ElementType& Front() const;
    NFE_INLINE ElementType& Front();

    /**
     * Get last element.
     * @note Array must not be empty. Otherwise it will cause an assertion.
     */
    NFE_INLINE const ElementType& Back() const;
    NFE_INLINE ElementType& Back();

    /**
     * Get iterator to the first element.
     */
    NFE_INLINE ConstIterator Begin() const;
    NFE_INLINE Iterator Begin();

    /**
     * Get iterator to the end.
     */
    NFE_INLINE ConstIterator End() const;
    NFE_INLINE Iterator End();

    /**
     * Element access operators.
     * @note The index must be valid. Otherwise it will cause an assertion.
     */
    NFE_INLINE ElementType& operator [] (uint32 index);
    NFE_INLINE const ElementType& operator [] (uint32 index) const;

    /**
     * Remove all the elements.
     */
    void Clear();

    /**
     * Insert a new element at the end.
     * @return  Iterator to the inserted element, or iterator to the end if the insertion failed.
     */
    Iterator PushBack(const ElementType& element);
    Iterator PushBack(ElementType&& element);

    /**
     * In-place construct a new element at the end.
     * @return  Iterator to the inserted element, or iterator to the end if the insertion failed.
     */
    template<typename ... Args>
    Iterator EmplaceBack(Args&& ... args);

    /**
     * Insert elements from a view at the end.
     * @return  'True' on success, 'false' on memory allocation failure.
     */
    template<typename ElementType2>
    bool PushBackArray(const ArrayView<ElementType2>& arrayView);

    /**
     * Insert a new element at given index.
     * @return  Iterator to the inserted element, or iterator to the end if the insertion failed.
     */
    Iterator InsertAt(uint32 index, const ElementType& element);
    Iterator InsertAt(uint32 index, ElementType&& element);

    /**
     * Insert a new element multiple times, at given index.
     * @return  Iterator to the first inserted element, or iterator to the end if the insertion failed.
     */
    Iterator InsertAt(uint32 index, const ElementType& element, uint32 count);

    /**
     * Insert elements from a view at given index.
     * @param   index   Place, where elements will be inserted. First element in the view will have
     *                  this index after the operation.
     * @param   arrayView Array view to insert.
     * @return  Iterator to the inserted element, or iterator to the end if the insertion failed.
     */
    template<typename ElementType2>
    Iterator InsertArrayAt(uint32 index, const ArrayView<ElementType2>& arrayView);

    /**
     * Remove an element by iterator.
     * @return  True if the element has been removed.
     */
    bool Erase(const ConstIterator& iterator);

    /**
     * Remove range of elements.
     * @return  True if the element has been removed.
     */
    bool Erase(const ConstIterator& first, const ConstIterator& last);

    // TODO erase with index table

    /**
     * Remove last element if exists.
     */
    bool PopBack();

    /**
     * Resize the array.
     * Element type must have default constructor.
     */
    bool Resize(uint32 size);

    /**
     * Resize the array and fill with a given template element.
     * Element type must have default constructor.
     */
    bool Resize(uint32 size, const ElementType& defaultElement);

    // lower-case aliases for Begin()/End(), required by C++ for range-based 'for' to work
    NFE_INLINE ConstIterator begin() const { return this->Begin(); }
    NFE_INLINE ConstIterator cbegin() const { return this->Begin(); }
    NFE_INLINE ConstIterator end() const { return this->End(); }
    NFE_INLINE ConstIterator cend() const { return this->End(); }
    NFE_INLINE Iterator begin() { return this->Begin(); }
    NFE_INLINE Iterator end() { return this->End(); }

private:

    // number of elements in the array
    uint32 mSize;

    static constexpr size_t ElementAlignment = alignof(ElementType);
    static constexpr size_t BufferSize = MaxSize * sizeof(ElementType);

    alignas(ElementAlignment) uint8 mData[BufferSize];
};


/**
 * Calculate hash of a static array.
 */
template<typename ElementType, uint32 MaxSize>
NFE_INLINE uint32 GetHash(const StaticArray<ElementType, MaxSize>& array)
{
    return GetHash(array.GetView());
}


} // namespace Common
} // namespace NFE


// StaticArray class definitions go here:
#include "StaticArrayImpl.hpp"
