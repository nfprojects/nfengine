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
public:
    class ConstIterator
    {
        friend class ArrayView;
    public:
        NFE_INLINE ConstIterator();
        NFE_INLINE bool operator == (const ConstIterator& other) const;
        NFE_INLINE bool operator != (const ConstIterator& other) const;
        NFE_INLINE const ElementType& operator*() const;
        NFE_INLINE ConstIterator& operator++();
        NFE_INLINE ConstIterator operator++(int);
    private:
        NFE_INLINE ConstIterator(ArrayView* array, uint32 index);
        ArrayView* mArray;  // array we are iterating
        uint32 mIndex;  // current node ID
    };

    class Iterator : public ConstIterator
    {
    public:
        NFE_INLINE ElementType& operator*();
    };


    // create empty view
    NFE_INLINE ArrayView();

    // create view of raw array
    NFE_INLINE ArrayView(ElementType* elements, uint32 numElements);

    NFE_INLINE ArrayView(const ArrayView& other);
    NFE_INLINE ArrayView(ArrayView&& other);
    NFE_INLINE ArrayView& operator = (const ArrayView& other);
    NFE_INLINE ArrayView& operator = (ArrayView&& other);

    /**
     * Get number of elements.
     */
    NFE_INLINE uint32 Size() const;

    /**
     * Maximum possible size (assuming infinite amount of memory).
     */
    static uint32 MaxSize();

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
     * Find element by value.
     * @note    This operation takes O(N) time.
     * @return  Iterator to the found element, or iterator to the end if the element does not exist.
     */
    ConstIterator Find(const ElementType& element) const;
    Iterator Find(const ElementType& element);

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
