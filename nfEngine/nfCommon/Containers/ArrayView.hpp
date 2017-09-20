/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  ArrayView declaration.
 */

#pragma once

#include "../nfCommon.hpp"

#include <iterator>

namespace NFE {
namespace Common {

/**
 * A view of contiguous array of elements.
 * Access type (const vs. non-const) depends on ElementType.
 */
template<typename ElementType>
class ArrayView
{
    template<typename T> friend class DynArray;

public:
    class ConstIterator : public std::iterator<std::bidirectional_iterator_tag, ElementType, int32>
    {
        friend class ArrayView;
        template<typename T> friend class DynArray;

    public:
        // C++ standard iterator traits
        using self_type = ConstIterator;

        // creates invalid iterator
        ConstIterator() : mArray(nullptr), mIndex(0) { }

        // comparisons
        NFE_INLINE bool operator == (const ConstIterator& other) const;
        NFE_INLINE bool operator != (const ConstIterator& other) const;
        NFE_INLINE bool operator < (const ConstIterator& rhs) const;
        NFE_INLINE int32 operator - (const ConstIterator& rhs) const;

        // element access
        NFE_INLINE const ElementType& operator*() const;
        NFE_INLINE const ElementType* operator->() const;

        // arithmetics
        NFE_INLINE ConstIterator& operator++();
        NFE_INLINE ConstIterator operator++(int);
        NFE_INLINE ConstIterator& operator+=(int32 offset);
        NFE_INLINE ConstIterator& operator--();
        NFE_INLINE ConstIterator operator--(int);
        NFE_INLINE ConstIterator& operator-=(int32 offset);
        NFE_INLINE ConstIterator operator+(int32 offset) const;
        NFE_INLINE ConstIterator operator-(int32 offset) const;

        // get array index
        int32 GetIndex() const { return mIndex; }

    protected:
        ConstIterator(const ArrayView* array, int32 index)
            : mArray(array), mIndex(index)
        { }

        const ArrayView* mArray;    // array we are iterating
        int32 mIndex;              // current index in the array
    };

    class Iterator : public ConstIterator
    {
        friend class ArrayView;
        template<typename T> friend class DynArray;

    public:
        // C++ standard iterator traits
        using self_type = Iterator;

        // creates invalid iterator
        Iterator() : ConstIterator() { }

        // comparisons
        NFE_INLINE int32 operator - (const Iterator& rhs) const;

        // element access
        NFE_INLINE ElementType& operator*() const;
        NFE_INLINE ElementType* operator->() const;

        // arithmetics
        NFE_INLINE Iterator& operator++();
        NFE_INLINE Iterator operator++(int);
        NFE_INLINE Iterator& operator+=(int32 offset);
        NFE_INLINE Iterator& operator--();
        NFE_INLINE Iterator operator--(int);
        NFE_INLINE Iterator& operator-=(int32 offset);
        NFE_INLINE Iterator operator+(int32 offset) const;
        NFE_INLINE Iterator operator-(int32 offset) const;

    private:
        // creates invalid iterator
        Iterator(const ArrayView* array, uint32 index) : ConstIterator(array, index) { }
    };


    // create empty view
    NFE_INLINE ArrayView();

    // create view of raw array
    NFE_INLINE ArrayView(ElementType* elements, uint32 numElements);

    // copy/move constructor/assignment
    ArrayView(const ArrayView& other) = default;
    ArrayView& operator = (const ArrayView& other) = default;
    ArrayView(ArrayView&& other) = default;
    ArrayView& operator = (ArrayView&& other) = default;

    // copy constructor from read-write-typed to const-typed
    template<typename ElementType2>
    ArrayView(const ArrayView<ElementType2>& other);

    // copy assignment from read-write-typed to const-typed
    template<typename ElementType2>
    ArrayView& operator = (const ArrayView<ElementType2>& other);


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
    NFE_INLINE ElementType& operator[](uint32 index);
    NFE_INLINE const ElementType& operator[](uint32 index) const;

    /**
     * Create view of a sub-range.
     * @param index Starting index.
     * @param size  Number of elements in range.
     */
    NFE_INLINE ArrayView Range(uint32 index, uint32 size) const;

    /**
     * Find element by value.
     * @note    This operation takes O(N) time.
     * @return  Iterator to the found element, or iterator to the end if the element does not exist.
     */
    ConstIterator Find(const ElementType& element) const;
    Iterator Find(const ElementType& element);

    // lower-case aliases for Begin()/End(), required by C++
    NFE_INLINE ConstIterator begin() const { return Begin(); }
    NFE_INLINE ConstIterator end() const { return End(); }
    NFE_INLINE ConstIterator cbegin() const { return Begin(); }
    NFE_INLINE ConstIterator cend() const { return End(); }
    NFE_INLINE Iterator begin() { return Begin(); }
    NFE_INLINE Iterator end() { return End(); }

protected:
    ElementType* mElements;
    uint32 mSize;
};

/**
 * Equality operator.
 */
template<typename ElementTypeA, typename ElementTypeB>
bool operator == (const ArrayView<ElementTypeA>& a, const ArrayView<ElementTypeB>& b);

/**
 * Inequality operator.
 */
template<typename ElementTypeA, typename ElementTypeB>
bool operator != (const ArrayView<ElementTypeA>& a, const ArrayView<ElementTypeB>& b);


} // namespace Common
} // namespace NFE

// ArrayView class definitions go here:
#include "ArrayViewImpl.hpp"
