/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Dynamic array (like std::vector) declarations
 */

#pragma once

#include "../nfCommon.hpp"
#include "ArrayView.hpp"
#include "../Memory/DefaultAllocator.hpp"
#include "../Logger.hpp"


namespace NFE {
namespace Common {

/**
 * Dynamic array (like std::vector).
 */
template<typename ElementType>
class DynArray : public ArrayView<ElementType>
{
public:
    // basic constructors and assignment operators
    NFE_INLINE DynArray();
    NFE_INLINE ~DynArray();
    NFE_INLINE DynArray(const DynArray& other);
    NFE_INLINE DynArray(DynArray&& other);
    NFE_INLINE DynArray& operator = (const DynArray& other);
    NFE_INLINE DynArray& operator = (DynArray&& other);

    // initialize using initializer list
    DynArray(const std::initializer_list<ElementType>& list);

    // initialize using C-style array
    DynArray(const ElementType* elements, uint32 count);

    /**
     * Remove all the elements.
     * @param freeMemory    Release memory?
     */
    void Clear(bool freeMemory = false);

    /**
     * Insert a new element at the end.
     * @return  Iterator to the inserted element, or iterator to the end if the insertion failed.
     */
    Iterator PushBack(const ElementType& element);
    Iterator PushBack(ElementType&& element);

    /**
     * Insert elements from a view at the end.
     * @return  'True' on success, 'false' on memory allocation failure.
     */
    bool PushBackArray(const ArrayView& arrayView);

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
    Iterator InsertArrayAt(uint32 index, const ArrayView& arrayView);

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

    // TODO erase range

    // TODO erase with index table

    /**
     * Remove last element if exists.
     */
    bool PopBack();

    /**
     * Reserve space.
     * @return  'false' if memory allocation failed.
     */
    bool Reserve(uint32 size);

    /**
     * Resize the array.
     * Element type must have default constructor.
     */
    bool Resize(uint32 size);

    /**
     * Replace contents of two arrays.
     * @note Does not call any constructor or destructor, just pointers are swapped.
     */
    void Swap(DynArray& other);

    // lower-case aliases for Begin()/End(), required by C++ for range-based 'for' to work
    NFE_INLINE ConstIterator begin() const { return Begin(); }
    NFE_INLINE ConstIterator end() const { return End(); }
    NFE_INLINE Iterator begin() { return Begin(); }
    NFE_INLINE Iterator end() { return End(); }

private:
    // allocated size
    uint32 mAllocSize;
};

} // namespace Common
} // namespace NFE

// DynArray class definitions go here:
#include "DynArrayImpl.hpp"
