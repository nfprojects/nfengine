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
class ConstArrayIterator : public std::iterator<std::bidirectional_iterator_tag, ElementType>
{
public:
    // C++ standard iterator traits
    using self_type = ConstArrayIterator;

    ConstArrayIterator() : mElements(nullptr), mIndex(0) { }
    ConstArrayIterator(const ElementType* elements, int32 index) : mElements(elements), mIndex(index) { }

    NFE_INLINE ConstArrayIterator(const ArrayIterator<const ElementType>& other);
    NFE_INLINE ConstArrayIterator(const ConstArrayIterator<const ElementType>& other);

    // comparisons
    NFE_INLINE bool operator == (const ConstArrayIterator& other) const;
    NFE_INLINE bool operator != (const ConstArrayIterator& other) const;
    NFE_INLINE bool operator < (const ConstArrayIterator& rhs) const;
    NFE_INLINE ptrdiff_t operator - (const ConstArrayIterator& rhs) const;

    // element access
    NFE_INLINE const ElementType& operator * () const;
    NFE_INLINE const ElementType* operator -> () const;

    // arithmetics
    NFE_INLINE ConstArrayIterator& operator ++ ();
    NFE_INLINE ConstArrayIterator operator ++ (int);
    NFE_INLINE ConstArrayIterator& operator += (ptrdiff_t offset);
    NFE_INLINE ConstArrayIterator& operator -- ();
    NFE_INLINE ConstArrayIterator operator -- (int);
    NFE_INLINE ConstArrayIterator& operator -= (ptrdiff_t offset);
    NFE_INLINE ConstArrayIterator operator + (ptrdiff_t offset) const;
    NFE_INLINE ConstArrayIterator operator - (ptrdiff_t offset) const;

    // get array index
    int32 GetIndex() const { return mIndex; }

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

    ArrayIterator() : ConstArrayIterator() { }
    ArrayIterator(ElementType* elements, uint32 index) : ConstArrayIterator(const_cast<ElementType*>(elements), index) { }

    // comparisons
    NFE_INLINE ptrdiff_t operator - (const ArrayIterator& rhs) const;

    // element access
    NFE_INLINE ElementType& operator * () const;
    NFE_INLINE ElementType* operator -> () const;

    // arithmetics
    NFE_INLINE ArrayIterator& operator ++ ();
    NFE_INLINE ArrayIterator operator ++ (int);
    NFE_INLINE ArrayIterator& operator += (ptrdiff_t offset);
    NFE_INLINE ArrayIterator& operator -- ();
    NFE_INLINE ArrayIterator operator -- (int);
    NFE_INLINE ArrayIterator& operator -= (ptrdiff_t offset);
    NFE_INLINE ArrayIterator operator + (ptrdiff_t offset) const;
    NFE_INLINE ArrayIterator operator - (ptrdiff_t offset) const;

    NFE_INLINE ElementType* GetElements() const { return const_cast<ElementType*>(mElements); }
};


} // namespace Common
} // namespace NFE

#include "ArrayIteratorImpl.hpp"