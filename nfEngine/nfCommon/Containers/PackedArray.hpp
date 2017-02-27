/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  PackedArray template class declaration
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Memory/DefaultAllocator.hpp"
#include "../Math/Math.hpp"

#include <limits>
#include <algorithm>
#include <functional>

namespace NFE {
namespace Common {

/**
 * PackedArray is a class that holds all objects in a contiguous memory block, even if they are
 * created and removed in a random order. PackedArray<ObjType, IDType> provides similar
 * functionality as std::map<IDType, ObjType> (where IDType is unsigned integer type),
 * but all operations have O(1) average complexity.
 *
 * @p ObjType describes objects types hold in the array.
 * @p IDType describes index type used to access the objects. It allows to choose an arbitrary
 * unsigned index size (unsigned char, unsigned short, unsigned int, etc.) in order to reduce
 * indices list memory footprint and gain additional performance (at the expense of lower
 * capacity).
 */
template<typename ObjType, typename IDType = unsigned int, size_t Alignment = 1>
class PackedArray
{
    static_assert(std::numeric_limits<IDType>::is_integer, "IDType must be integer type");
    static_assert(!std::numeric_limits<IDType>::is_signed, "IDType must be unsigned type");
    static_assert(Math::PowerOfTwo(Alignment), "'Alignment' template parameter must be a power of two.");

    // doubly link list node
    struct ListNode
    {
        IDType next;
        IDType prev;
    };

    size_t mSize;        //< total allocated space (in objects)
    size_t mUsed;        //< created objects
    IDType* mIDs;        //< array of object IDs
    ListNode* mNodes;    //< list nodes for free and taken objects
    ObjType* mObjects;   //< block of allocated objects
    IDType mFreeHead;    //< head of free objects list
    IDType mTakenHead;   //< head of taken objects list

    /// we don't want these
    PackedArray(const PackedArray&) = delete;
    PackedArray(PackedArray&&) = delete;
    PackedArray& operator=(const PackedArray&) = delete;
    PackedArray& operator=(PackedArray&&) = delete;

    bool Resize(size_t newSize);

public:
    const IDType InvalidIndex = static_cast<IDType>(-1);
    typedef std::function<void(ObjType&)> IteratorCallback;

    PackedArray(size_t initialSize = 64);
    ~PackedArray();

    /**
     * Add an object to the array (by reference).
     * @param obj Object to be added
     * @return    Object index
     */
    IDType Add(const ObjType& obj);

    /**
     * Add an object to the array (by rvalue reference).
     * @param obj Object to be added
     * @return    Object index
     */
    IDType Add(ObjType&& obj);

    /**
     * Check if provided index is a valid index.
     */
    bool Has(IDType index);

    /**
     * Remove an object from the array.
     * @param index Index of an object to be removed
     */
    void Remove(IDType index);

    /**
     * Object access.
     * @param index Object index.
     * @return Object reference.
     */
    NFE_INLINE ObjType& operator[](IDType index);

    /**
     * Object access.
     * @param index Object index.
     * @return Constant object reference.
     */
    NFE_INLINE const ObjType& operator[](IDType index) const;

    /**
     * Iterate through all the objects the fastest possible way.
     * @param func Function to be called for each object.
     */
    void Iterate(const IteratorCallback& func) const;

    /**
     * Get number of objects in the array.
     */
    NFE_INLINE size_t Size() const;

    /**
     * Get maximum array size (in objects count) with given @p IDType.
     */
    NFE_INLINE static size_t MaxSize();

    /**
     * Print freelist structure for debugging purposes.
     */
    void DebugPrint();

private:
    IDType AddInternal(IDType& id);
};

} // namespace Common
} // namespace NFE


// PackedArray definitions go here:
#include "PackedArrayImpl.hpp"
