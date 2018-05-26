/**
 * @file
 * @author Witek902
 * @brief  Dynamic bidirectional queue (like std::deque) declarations
 */

#pragma once

#include "../nfCommon.hpp"
#include "ArrayView.hpp"
#include "../Memory/DefaultAllocator.hpp"
#include "../Logger/Logger.hpp"


namespace NFE {
namespace Common {

template<typename ElementType>
class Deque
{
public:
    NFE_INLINE Deque();
    NFE_INLINE explicit Deque(const uint32 initialCapacity);
    NFE_INLINE ~Deque();
    NFE_INLINE Deque(const Deque& other);
    NFE_INLINE Deque(Deque&& other);
    NFE_INLINE Deque& operator = (const Deque& other);
    NFE_INLINE Deque& operator = (Deque&& other);

    /**
     * Remove all the elements.
     * @param freeMemory    Release memory?
     */
    void Clear(bool freeMemory = false);

    /**
     * Get numer of elements in the deque.
     */
    uint32 Size() const;

    /**
     * Check if the container is empty.
     */
    bool Empty() const;

    /**
     * Access front element.
     */
    const ElementType& Front() const;
    ElementType& Front();

    /**
     * Access back element.
     */
    const ElementType& Back() const;
    ElementType& Back();

    /**
     * Insert a new element at the end.
     */
    bool PushBack(const ElementType& element);
    bool PushBack(ElementType&& element);

    /**
     * In-place construct a new element at the end.
     */
    template<typename ... Args>
    bool EmplaceBack(Args&& ... args);

    /**
     * Insert a new element at the front.
     */
    bool PushFront(const ElementType& element);
    bool PushFront(ElementType&& element);

    /**
     * In-place construct a new element at the front.
     */
    template<typename ... Args>
    bool EmplaceFront(Args&& ... args);

    /**
     * Remove last element (if exists).
     * @return 'false' if there is nothing to pop (container is empty).
     */
    bool PopBack();

    /**
     * Remove first element (if exists).
     * @return 'false' if there is nothing to pop (container is empty).
     */
    bool PopFront();

    /**
     * Reserve space.
     * @return  'false' if memory allocation failed.
     */
    bool Reserve(uint32 size);

    /**
     * Replace contents of two deques.
     * @note Does not call any constructor or destructor, just pointers are swapped.
     */
    void Swap(Deque& other);

private:
    bool ContainsElement(const ElementType& element) const;
    uint32 GetElementIndex(uint32 index) const;

    ElementType* mElements;
    uint32 mCapacity;   // allocated size
    uint32 mHead;       // index of a first element
    uint32 mSize;       // number of elements (always power of two or zero)
};


} // namespace Common
} // namespace NFE

// Deque class definitions go here:
#include "DequeImpl.hpp"
