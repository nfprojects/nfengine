/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Hash set data container definitions
 */

#pragma once

#include "HashSet.hpp"
#include "../System/Assertion.hpp"
#include "../Memory/MemoryHelpers.hpp"


namespace NFE {
namespace Common {


// HashSet::ConstIterator /////////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::ConstIterator::ConstIterator()
    : mSet(nullptr)
    , mBucket(InvalidID)
    , mElement(InvalidID)
{ }

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::ConstIterator::ConstIterator(const HashSet* set, BucketID bucket, ElementID element)
    : mSet(set)
    , mBucket(bucket)
    , mElement(element)
{ }

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::ConstIterator::operator == (const ConstIterator& other) const
{
    // note: no need to compare bucket ID, because element ID identifies the key uniquely
    return (mSet == other.mSet) && (mElement == other.mElement);
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::ConstIterator::operator != (const ConstIterator& other) const
{
    return (mSet != other.mSet) || (mElement != other.mElement);
}

template<typename KeyType, typename HashFunction>
const KeyType& HashSet<KeyType, HashFunction>::ConstIterator::operator*() const
{
    NFE_ASSERT(mElement != InvalidID, "Trying to dereference 'end' iterator");
    return mSet->mKeys[mElement];
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::ConstIterator& HashSet<KeyType, HashFunction>::ConstIterator::operator++()
{
    // TODO
    // go to next element
    // if there is no more elements in the bucket, go to the next bucket

    return *this;
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::ConstIterator HashSet<KeyType, HashFunction>::ConstIterator::operator++(int)
{
    ConstIterator tmp(*this);
    operator++();
    return tmp;
}


// HashSet::Iterator //////////////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::Iterator::Iterator(const HashSet* set, BucketID bucket, ElementID element)
    : ConstIterator(set, bucket, element)
{ }

template<typename KeyType, typename HashFunction>
KeyType& HashSet<KeyType, HashFunction>::Iterator::operator*() const
{
    NFE_ASSERT(this->mElement != InvalidID, "Trying to dereference 'end' iterator");
    return this->mSet->mKeys[this->mElement];
}


// HashSet ////////////////////////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::HashSet()
    : mKeys(nullptr)
    , mNextElements(nullptr)
    , mBuckets(nullptr)
    , mNumElements(0)
    , mAllocatedSize(0)
    , mHashBits(0)
{
    // TODO
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::~HashSet()
{
    Clear();
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::HashSet(const HashSet& other)
{
    // TODO
    UNUSED(other);
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>& HashSet<KeyType, HashFunction>::operator = (const HashSet& other)
{
    // TODO
    UNUSED(other);

    return *this;
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::HashSet(HashSet&& other)
{
    mKeys = other.mKeys;
    mNextElements = other.mNextElements;
    mBuckets = other.mBuckets;
    mNumElements = other.mNumElements;
    mAllocatedSize = other.mAllocatedSize;
    mHashBits = other.mHashBits;

    other.mKeys = nullptr;
    other.mNextElements = nullptr;
    other.mBuckets = nullptr;
    other.mNumElements = 0;
    other.mAllocatedSize = 0;
    other.mHashBits = 0;
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>& HashSet<KeyType, HashFunction>::operator = (HashSet&& other)
{
    // TODO reuse already allocated memory
    Clear();

    mKeys = other.mKeys;
    mNextElements = other.mNextElements;
    mBuckets = other.mBuckets;
    mNumElements = other.mNumElements;
    mAllocatedSize = other.mAllocatedSize;
    mHashBits = other.mHashBits;

    other.mKeys = nullptr;
    other.mNextElements = nullptr;
    other.mBuckets = nullptr;
    other.mNumElements = 0;
    other.mAllocatedSize = 0;
    other.mHashBits = 0;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename HashFunction>
uint32 HashSet<KeyType, HashFunction>::Size() const
{
    return mNumElements;
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Empty() const
{
    return mNumElements == 0;
}

template<typename KeyType, typename HashFunction>
uint32 HashSet<KeyType, HashFunction>::GetNumberOfBuckets() const
{
    return mHashBits > 0 ? (1 << (static_cast<uint32>(mHashBits) - 1)) : 0;
}

template<typename KeyType, typename HashFunction>
uint32 HashSet<KeyType, HashFunction>::GetHashMask() const
{
    return (1 << static_cast<uint32>(mHashBits)) - 1;
}

template<typename KeyType, typename HashFunction>
void HashSet<KeyType, HashFunction>::Clear()
{
    NFE_FREE(mKeys);
    NFE_FREE(mNextElements);
    NFE_FREE(mBuckets);

    mKeys = nullptr;
    mNextElements = nullptr;
    mBuckets = nullptr;
    mNumElements = 0;
    mAllocatedSize = 0;
    mHashBits = 0;
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::ConstIterator HashSet<KeyType, HashFunction>::Begin() const
{
    BucketID bucketID = InvalidID;
    ElementID elementID = InvalidID;
    FindFirstElement(bucketID, elementID);

    return ConstIterator(this, bucketID, elementID);
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::Iterator HashSet<KeyType, HashFunction>::Begin()
{
    BucketID bucketID = InvalidID;
    ElementID elementID = InvalidID;
    FindFirstElement(bucketID, elementID);

    return Iterator(this, bucketID, elementID);
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::ConstIterator HashSet<KeyType, HashFunction>::End() const
{
    return ConstIterator(this, InvalidID, InvalidID);
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::Iterator HashSet<KeyType, HashFunction>::End()
{
    return Iterator(this, InvalidID, InvalidID);
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::ConstIterator HashSet<KeyType, HashFunction>::Find(const KeyType& key) const
{
    BucketID bucketID = InvalidID;
    ElementID elementID = InvalidID;
    FindKeyInternal(key, bucketID, elementID);

    return ConstIterator(this, bucketID, elementID);
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::Iterator HashSet<KeyType, HashFunction>::Find(const KeyType& key)
{
    BucketID bucketID = InvalidID;
    ElementID elementID = InvalidID;
    FindKeyInternal(key, bucketID, elementID);

    return Iterator(this, bucketID, elementID);
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::InsertResult HashSet<KeyType, HashFunction>::Insert(const KeyType& key)
{
    // TODO
    UNUSED(key);

    return InsertResult(End());
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::InsertResult HashSet<KeyType, HashFunction>::Insert(KeyType&& key)
{
    // TODO
    UNUSED(key);

    return InsertResult(End());
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Erase(const KeyType& key)
{
    BucketID bucketID = InvalidID;
    ElementID elementID = InvalidID;
    if (!FindKeyInternal(key, bucketID, elementID))
    {
        return false;
    }

    // TODO
    // 1. destroy key
    // 2. update linked list for the bucket

    return true;
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Erase(const ConstIterator& iterator)
{
    NFE_ASSERT(iterator.mSet == this, "Trying to use iterator from another HashSet");

    // TODO
    UNUSED(iterator);

    return false;
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Erase(const Iterator& iterator)
{
    NFE_ASSERT(iterator.mSet == this, "Trying to use iterator from another HashSet");

    // TODO
    UNUSED(iterator);

    return false;
}

template<typename KeyType, typename HashFunction>
void HashSet<KeyType, HashFunction>::DebugPrint() const
{
    // TODO
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Verify() const
{
    // TODO
    return false;
}

//////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::FindFirstElement(BucketID& outBucketId, ElementID& outElementId) const
{
    for (uint32 i = 0; i < GetNumberOfBuckets(); ++i)
    {
        if (mBuckets[i] != InvalidID)
        {
            outBucketId = i;
            outElementId = mBuckets[i];
            return true;
        }
    }

    return false;
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::FindKeyInternal(const KeyType& key, BucketID& outBucketId, ElementID& outElementId) const
{
    if (GetNumberOfBuckets() == 0)
    {
        return false;
    }

    HashFunction hashFunction;

    // calculate masked hash value (equals to bucket ID)
    const uint32 hashValue = hashFunction(key) & GetHashMask();
    NFE_ASSERT(hashValue < GetNumberOfBuckets(), "Invalid hash value. This indicates hashset bug");

    // search for the key in the linked list
    ElementID currentElement = mBuckets[hashValue];
    while (currentElement != InvalidID)
    {
        if (mKeys[currentElement] == key)
        {
            // key found
            outBucketId = hashValue;
            outElementId = currentElement;
            return true;
        }

        currentElement = mNextElements[currentElement];
    }

    // key not found
    return false;
}

} // namespace Common
} // namespace NFE
