/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Array iterators declarations.
 */

#pragma once

#include "../../nfCommon.hpp"

#include <iterator>

namespace NFE {
namespace Common {


template<typename ElementType>
class ArrayIterator;

/**
 * Array iterator with read-only access.
 */
template<typename ElementType>
class ConstArrayIterator
{
    template<typename T>
    friend class ConstArrayIterator;

public:
    // C++ standard iterator traits
    using self_type = ConstArrayIterator;
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = ElementType;
	using difference_type = std::ptrdiff_t;
	using pointer = ElementType*;
	using reference = ElementType&;

    NFE_FORCE_INLINE ConstArrayIterator() : mElements(nullptr), mIndex(0) { }
    NFE_FORCE_INLINE ConstArrayIterator(const ElementType* elements, int32 index)
        : mElements(elements), mIndex(index)
    { }

    NFE_FORCE_INLINE ConstArrayIterator(const ArrayIterator<const ElementType>& other);
    NFE_FORCE_INLINE ConstArrayIterator(const ConstArrayIterator<const ElementType>& other);

    // comparisons
    NFE_FORCE_INLINE bool operator == (const ConstArrayIterator& other) const;
    NFE_FORCE_INLINE bool operator != (const ConstArrayIterator& other) const;
    NFE_FORCE_INLINE bool operator < (const ConstArrayIterator& rhs) const;
    NFE_FORCE_INLINE ptrdiff_t operator - (const ConstArrayIterator& rhs) const;

    // element access
    NFE_FORCE_INLINE const ElementType& operator * () const;
    NFE_FORCE_INLINE const ElementType* operator -> () const;
    NFE_FORCE_INLINE const ElementType& operator [] (ptrdiff_t index) const;

    // arithmetics
    NFE_FORCE_INLINE ConstArrayIterator& operator ++ ();
    NFE_FORCE_INLINE ConstArrayIterator operator ++ (int);
    NFE_FORCE_INLINE ConstArrayIterator& operator += (ptrdiff_t offset);
    NFE_FORCE_INLINE ConstArrayIterator& operator -- ();
    NFE_FORCE_INLINE ConstArrayIterator operator -- (int);
    NFE_FORCE_INLINE ConstArrayIterator& operator -= (ptrdiff_t offset);
    NFE_FORCE_INLINE ConstArrayIterator operator + (ptrdiff_t offset) const;
    NFE_FORCE_INLINE ConstArrayIterator operator - (ptrdiff_t offset) const;

    // get array index
    NFE_FORCE_INLINE int32 GetIndex() const { return mIndex; }

protected:
    const ElementType* mElements;   // array elements
    int32 mIndex;                   // current index in the array
};

/**
 * Array iterator with read-write access.
 */
template<typename ElementType>
class ArrayIterator : public ConstArrayIterator<ElementType>
{
public:
    // C++ standard iterator traits
    using self_type = ArrayIterator;

    NFE_FORCE_INLINE ArrayIterator() = default;
    NFE_FORCE_INLINE ArrayIterator(ElementType* elements, uint32 index)
        : ConstArrayIterator<ElementType>(const_cast<ElementType*>(elements), index)
    { }

    // comparisons
    NFE_FORCE_INLINE ptrdiff_t operator - (const ArrayIterator& rhs) const;

    // element access
    NFE_FORCE_INLINE ElementType& operator * () const;
    NFE_FORCE_INLINE ElementType* operator -> () const;
    NFE_FORCE_INLINE ElementType& operator [] (ptrdiff_t index) const;

    // arithmetics
    NFE_FORCE_INLINE ArrayIterator& operator ++ ();
    NFE_FORCE_INLINE ArrayIterator operator ++ (int);
    NFE_FORCE_INLINE ArrayIterator& operator += (ptrdiff_t offset);
    NFE_FORCE_INLINE ArrayIterator& operator -- ();
    NFE_FORCE_INLINE ArrayIterator operator -- (int);
    NFE_FORCE_INLINE ArrayIterator& operator -= (ptrdiff_t offset);
    NFE_FORCE_INLINE ArrayIterator operator + (ptrdiff_t offset) const;
    NFE_FORCE_INLINE ArrayIterator operator - (ptrdiff_t offset) const;

    NFE_FORCE_INLINE ElementType* GetElements() const { return const_cast<ElementType*>(this->mElements); }
};


} // namespace Common
} // namespace NFE

#include "ArrayIteratorImpl.hpp"