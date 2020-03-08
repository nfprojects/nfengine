/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  ArrayView declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "Hash.hpp"
#include "Iterators/ArrayIterator.hpp"


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

    // maximum size of array view (1 element is reserved for iterator to the end)
    static constexpr uint32 MaxSize = std::numeric_limits<uint32>::max() - 1;

    using Iterator = ArrayIterator<ElementType>;
    using ConstIterator = ConstArrayIterator<ElementType>;

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
 * Comparison operators.
 */
template<typename ElementTypeA, typename ElementTypeB>
bool operator == (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator != (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator < (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator > (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator <= (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);
template<typename ElementTypeA, typename ElementTypeB>
bool operator >= (const ArrayView<ElementTypeA>& lhs, const ArrayView<ElementTypeB>& rhs);


/**
 * Calculate hash of an array view.
 * @note This function is meant to be fast (it's used in hash tables), not to be cryptographically secure.
 */
template<typename ElementType>
uint32 GetHash(const ArrayView<ElementType>& arrayView);


} // namespace Common
} // namespace NFE

// ArrayView class definitions go here:
#include "ArrayViewImpl.hpp"
