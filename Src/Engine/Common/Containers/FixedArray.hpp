/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Fixed array container declarations
 */

#pragma once

#include "../nfCommon.hpp"
#include "ArrayView.hpp"

#include <iterator>


namespace NFE {
namespace Common {

/**
 * Fixed array - fixed-size array, cannot be resized (like std::array).
 * Mainly used for runtime range checks.
 */
template<typename ElementType, uint32 ArraySize>
class FixedArray final
{
public:
    static_assert(ArraySize > 0, "Fixed array's size must be greater than zero");

    using Iterator = typename ArrayView<ElementType>::Iterator;
    using ConstIterator = typename ArrayView<ElementType>::ConstIterator;

    // basic constructors and assignment operators
    NFE_INLINE FixedArray() = default;
    NFE_INLINE ~FixedArray() = default;

    // TODO support 'other' of different size
    NFE_INLINE FixedArray(const ElementType& defaultElement);
    NFE_INLINE FixedArray(const FixedArray& other);
    NFE_INLINE FixedArray(FixedArray&& other);
    NFE_INLINE FixedArray& operator = (const FixedArray& other);
    NFE_INLINE FixedArray& operator = (FixedArray&& other);

    // initialize using initializer list
    NFE_INLINE FixedArray(const std::initializer_list<ElementType>& list);

    /**
     * Get a view of this array.
     */
    NFE_INLINE ArrayView<ElementType> GetView();
    NFE_INLINE ArrayView<const ElementType> GetView() const;

    /**
     * Get number of elements.
     */
    NFE_FORCE_INLINE static constexpr uint32 Size() { return ArraySize; }

    /**
     * Get raw data pointed by the view.
     */
    NFE_FORCE_INLINE const ElementType* Data() const { return mElements; }
    NFE_FORCE_INLINE ElementType* Data() { return mElements; }

    /**
     * Get first element.
     * @note Array must not be empty. Otherwise it will cause an assertion.
     */
    NFE_FORCE_INLINE const ElementType& Front() const;
    NFE_FORCE_INLINE ElementType& Front();

    /**
     * Get last element.
     * @note Array must not be empty. Otherwise it will cause an assertion.
     */
    NFE_FORCE_INLINE const ElementType& Back() const;
    NFE_FORCE_INLINE ElementType& Back();

    /**
     * Get iterator to the first element.
     */
    NFE_FORCE_INLINE ConstIterator Begin() const;
    NFE_FORCE_INLINE Iterator Begin();

    /**
     * Get iterator to the end.
     */
    NFE_FORCE_INLINE ConstIterator End() const;
    NFE_FORCE_INLINE Iterator End();

    /**
     * Element access operators.
     * @note The index must be valid. Otherwise it will cause an assertion.
     */
    NFE_FORCE_INLINE ElementType& operator [] (uint32 index);
    NFE_FORCE_INLINE const ElementType& operator [] (uint32 index) const;

    // lower-case aliases for Begin()/End(), required by C++ for range-based 'for' to work
    NFE_FORCE_INLINE ConstIterator begin() const { return this->Begin(); }
    NFE_FORCE_INLINE ConstIterator cbegin() const { return this->Begin(); }
    NFE_FORCE_INLINE ConstIterator end() const { return this->End(); }
    NFE_FORCE_INLINE ConstIterator cend() const { return this->End(); }
    NFE_FORCE_INLINE Iterator begin() { return this->Begin(); }
    NFE_FORCE_INLINE Iterator end() { return this->End(); }

private:
    ElementType mElements[ArraySize];
};


/**
 * Calculate hash of a static array.
 */
template<typename ElementType, uint32 ArraySize>
NFE_FORCE_INLINE uint32 GetHash(const FixedArray<ElementType, ArraySize>& array)
{
    return GetHash(array.GetView());
}


} // namespace Common
} // namespace NFE


// FixedArray class definitions go here:
#include "FixedArrayImpl.hpp"
