/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  ArrayView declaration.
 */

#pragma once

#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

/**
 * A view of contiguous array of elements.
 */
template<typename ElementType>
class ArrayView
{
    template<typename T> friend class DynArray;

public:
    class ConstIterator
    {
        friend class ArrayView;
        template<typename T> friend class DynArray;

    public:
        NFE_INLINE ConstIterator() : mArray(nullptr), mIndex(0) { }
        NFE_INLINE bool operator == (const ConstIterator& other) const;
        NFE_INLINE bool operator != (const ConstIterator& other) const;
        NFE_INLINE const ElementType& operator*() const;
        NFE_INLINE ConstIterator& operator++();
        NFE_INLINE ConstIterator operator++(int);
        NFE_INLINE ConstIterator operator+(int32 offset);
        NFE_INLINE ConstIterator operator-(int32 offset);
        NFE_INLINE uint32 GetIndex() const { return mIndex; }

    protected:
        NFE_INLINE ConstIterator(const ArrayView* array, uint32 index)
            : mArray(array), mIndex(index)
        { }

        const ArrayView* mArray;    // array we are iterating
        uint32 mIndex;              // current index in the array
    };

    class Iterator : public ConstIterator
    {
        friend class ArrayView;
        template<typename T> friend class DynArray;

    public:
        NFE_INLINE ElementType& operator*() const;

    private:
        NFE_INLINE Iterator(ArrayView* array, uint32 index)
            : ConstIterator(array, index)
        { }
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

    /**
     * Equality operators.
     */
    bool operator == (const ArrayView& other) const;
    bool operator != (const ArrayView& other) const;

    // lower-case aliases for Begin()/End(), required by C++
    NFE_INLINE ConstIterator begin() const { return Begin(); }
    NFE_INLINE ConstIterator end() const { return End(); }
    NFE_INLINE Iterator begin() { return Begin(); }
    NFE_INLINE Iterator end() { return End(); }

protected:
    ElementType* mElements;
    uint32 mSize;
};

} // namespace Common
} // namespace NFE

// ArrayView class definitions go here:
#include "ArrayViewImpl.hpp"
