/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  PackedArray template class definition
 */

#pragma once

#include "PackedArray.hpp"
#include "../System/Assertion.hpp"
#include "../Memory/MemoryHelpers.hpp"

#include <string.h>


namespace NFE {
namespace Common {

template<typename ObjType, typename IDType, size_t Alignment>
PackedArray<ObjType, IDType, Alignment>::PackedArray(size_t initialSize)
{
    mSize = 0;
    mUsed = 0;
    mIDs = nullptr;
    mNodes = nullptr;
    mObjects = nullptr;
    mFreeHead = InvalidIndex;
    mTakenHead = InvalidIndex;

    Resize(initialSize);
}

template<typename ObjType, typename IDType, size_t Alignment>
PackedArray<ObjType, IDType, Alignment>::~PackedArray()
{
    if (mNodes && mObjects && mIDs)
    {
        // call destructors
        for (size_t i = 0; i < mUsed; ++i)
        {
            mObjects[i].~ObjType();
        }
    }

    if (mNodes)
        NFE_FREE(mNodes);
    if (mObjects)
        NFE_FREE(mObjects);
    if (mIDs)
        NFE_FREE(mIDs);
}

template<typename ObjType, typename IDType, size_t Alignment>
bool PackedArray<ObjType, IDType, Alignment>::Resize(size_t newSize)
{
    NFE_ASSERT(newSize >= 4, "Packed array is too small");
    NFE_ASSERT(newSize <= MaxSize(), "Packed array is too big");

    ObjType* newObjects = static_cast<ObjType*>(NFE_MALLOC(sizeof(ObjType) * newSize, Alignment));
    ListNode* newNodes = static_cast<ListNode*>(NFE_MALLOC(sizeof(ListNode) * newSize, Alignment));
    IDType* newIDs = static_cast<IDType*>(NFE_MALLOC(sizeof(IDType) * newSize, Alignment));

    // allocation failed
    if (newNodes == nullptr || newObjects == nullptr || newIDs == nullptr)
    {
        NFE_FREE(newObjects);
        NFE_FREE(newNodes);
        NFE_FREE(newIDs);
        return false;
    }

    // move objects from old buffer to the new one
    for (size_t i = 0; i < mUsed; ++i)
    {
        MemoryHelpers::Move<ObjType>(newObjects + i, mObjects + i);
    }

    memcpy(newNodes, mNodes, sizeof(ListNode) * mSize);
    memcpy(newIDs, mIDs, sizeof(IDType) * mSize);

    NFE_FREE(mObjects);
    NFE_FREE(mNodes);
    NFE_FREE(mIDs);

    mObjects = newObjects;
    mNodes = newNodes;
    mIDs = newIDs;

    // build list of free object slots

    /// first index
    mIDs[mSize] = InvalidIndex;
    mNodes[mSize].next = static_cast<IDType>(mSize + 1);
    mNodes[mSize].prev = InvalidIndex;

    /// "middle"
    for (size_t i = mSize + 1; i < newSize - 1; ++i)
    {
        mIDs[i] = InvalidIndex;
        mNodes[i].next = static_cast<IDType>(i + 1);
        mNodes[i].prev = static_cast<IDType>(i - 1);
    }

    /// last index
    mIDs[newSize - 1] = InvalidIndex;
    mNodes[newSize - 1].next = static_cast<IDType>(mFreeHead);
    mNodes[newSize - 1].prev = static_cast<IDType>(newSize - 2);

    /// update pointers
    if (mFreeHead == InvalidIndex)
        mFreeHead = static_cast<IDType>(mSize);

    mSize = newSize;
    return true;
}

template<typename ObjType, typename IDType, size_t Alignment>
IDType PackedArray<ObjType, IDType, Alignment>::AddInternal(IDType& id)
{
    if (mUsed == MaxSize())
        return InvalidIndex;

    if (mFreeHead == InvalidIndex)
    {
        size_t newSize = std::min(MaxSize(), mSize * 2);
        if (!Resize(newSize))
            return InvalidIndex;
    }

    // get next free index and update the free list
    IDType index = mFreeHead;
    id = static_cast<IDType>(mUsed); // next free object is always at the end

    // update free indices list
    mFreeHead = mNodes[index].next;
    if (mFreeHead != InvalidIndex)
        mNodes[mFreeHead].prev = InvalidIndex;

    if (mTakenHead != InvalidIndex)
        mNodes[mTakenHead].prev = index;

    mIDs[index] = id;
    mNodes[index].next = mTakenHead;
    mNodes[index].prev = InvalidIndex;
    mTakenHead = index;

    mUsed++;

    return index;
}

template<typename ObjType, typename IDType, size_t Alignment>
IDType PackedArray<ObjType, IDType, Alignment>::Add(const ObjType& obj)
{
    IDType index, id = 0;
    index = AddInternal(id);
    if (index == InvalidIndex)
    {
        return InvalidIndex;
    }

    new (mObjects + id) ObjType(obj);
    return index;
}

template<typename ObjType, typename IDType, size_t Alignment>
IDType PackedArray<ObjType, IDType, Alignment>::Add(ObjType&& obj)
{
    IDType index, id = 0;
    index = AddInternal(id);
    if (index == InvalidIndex)
    {
        return InvalidIndex;
    }

    new (mObjects + id) ObjType(std::move(obj));
    return index;
}

template<typename ObjType, typename IDType, size_t Alignment>
bool PackedArray<ObjType, IDType, Alignment>::Has(IDType index)
{
    return
        (index >= 0) && (index < mSize) &&  // check if index is in range of index table
        (mIDs[index] < mUsed);  // check if index points to object in range of objects table
}

template<typename ObjType, typename IDType, size_t Alignment>
void PackedArray<ObjType, IDType, Alignment>::Remove(IDType index)
{
    NFE_ASSERT(Has(index), "Invalid index");

    IDType id = mIDs[index];
    --mUsed;
    if (id < mUsed)
    {
        // move the object from the end to fill the gap
        MemoryHelpers::Move<ObjType>(mObjects + id, mObjects + mUsed);
    }
    else
    {
        mObjects[id].~ObjType();
    }

    IDType newLastTaken = mNodes[mTakenHead].next;

    if (newLastTaken == index)
        newLastTaken = mTakenHead;

    if (mNodes[mTakenHead].next != InvalidIndex)
        mNodes[mNodes[mTakenHead].next].prev = InvalidIndex;

    /// update indicies of moved object
    mIDs[mTakenHead] = mIDs[index];
    mNodes[mTakenHead].next = mNodes[index].next;
    mNodes[mTakenHead].prev = mNodes[index].prev;

    /// correct neighbors pointers
    if (mNodes[mTakenHead].prev != InvalidIndex)
        mNodes[mNodes[mTakenHead].prev].next = mTakenHead;
    if (mNodes[mTakenHead].next != InvalidIndex)
        mNodes[mNodes[mTakenHead].next].prev = mTakenHead;

    mTakenHead = newLastTaken;

    // update free indices list
    mIDs[index] = InvalidIndex;
    mNodes[index].prev = InvalidIndex;
    mNodes[index].next = mFreeHead;
    mFreeHead = index;

    if (mTakenHead != InvalidIndex)
        NFE_ASSERT(mIDs[mTakenHead] == (mUsed - 1), "Free list corruption");
    NFE_ASSERT(mTakenHead != mFreeHead, "Free list corruption");
}

template<typename ObjType, typename IDType, size_t Alignment>
ObjType& PackedArray<ObjType, IDType, Alignment>::operator[](IDType index)
{
    NFE_ASSERT(Has(index), "Invalid index");
    return mObjects[mIDs[index]];
}

template<typename ObjType, typename IDType, size_t Alignment>
const ObjType& PackedArray<ObjType, IDType, Alignment>::operator[](IDType index) const
{
    NFE_ASSERT(Has(index), "Invalid index");
    return mObjects[mIDs[index]];
}

template<typename ObjType, typename IDType, size_t Alignment>
void PackedArray<ObjType, IDType, Alignment>::Iterate(const IteratorCallback& func) const
{
    for (size_t i = 0; i < mUsed; ++i)
        func(mObjects[i]);
}

template<typename ObjType, typename IDType, size_t Alignment>
size_t PackedArray<ObjType, IDType, Alignment>::Size() const
{
    return mUsed;
}

template<typename ObjType, typename IDType, size_t Alignment>
size_t PackedArray<ObjType, IDType, Alignment>::MaxSize()
{
    return static_cast<IDType>(-1);
}

template<typename ObjType, typename IDType, size_t Alignment>
void PackedArray<ObjType, IDType, Alignment>::DebugPrint()
{
    std::cout << "Usage: " << mUsed << "/" << mSize << std::endl;
    std::cout << "NextFree: " << mFreeHead << std::endl;
    std::cout << "LastTaken: " << mTakenHead << std::endl;

    for (size_t i = 0; i < mSize; ++i)
    {
        std::cout << i << ": "
            << "id = " << mIDs[i]
            << ", next = " << mNodes[i].next
            << ", prev = " << mNodes[i].prev << std::endl;
    }
}

} // namespace Common
} // namespace NFE
