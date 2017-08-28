/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Hash set data container definitions
 */

#pragma once

#include "HashSet.hpp"
#include "../System/Assertion.hpp"
#include "../Memory/MemoryHelpers.hpp"
#include "../Memory/DefaultAllocator.hpp"
#include "../Logger/Logger.hpp"


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
HashSet<KeyType, HashFunction>::ConstIterator::ConstIterator(const HashSet* set, ElementID bucket, ElementID element)
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
    if (mElement != InvalidID) // update only when not pointing to the end
    {
        const ElementID nextElement = mSet->mNextElements[mElement];
        if (nextElement == InvalidID) // linked list end for this bucket reached, go the the next bucket
        {
            bool nextBucketFound = false;
            const uint32 numBuckets = mSet->GetNumberOfBuckets();
            for (uint32 i = mBucket + 1; i < numBuckets; ++i) // TODO this is too slow...
            {
                const ElementID bucketHead = mSet->mBuckets[i];
                if (bucketHead != InvalidID)
                {
                    mBucket = i;
                    mElement = bucketHead;
                    nextBucketFound = true;
                    break;
                }
            }

            // no next bucket found - set end reached
            if (!nextBucketFound)
            {
                mBucket = InvalidID;
                mElement = InvalidID;
            }
        }
        else
        {
            mElement = nextElement;
        }
    }

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
HashSet<KeyType, HashFunction>::Iterator::Iterator(const HashSet* set, ElementID bucket, ElementID element)
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
    , mNumKeys(0)
    , mHashBits(0)
    , mFirstFreeElement(InvalidID)
    , mFirstFreeKey(InvalidID)
{
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::~HashSet()
{
    Clear();
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::HashSet(const HashSet& other)
    : HashSet()
{
    Reserve(other.Size());

    for (KeyType& key : other)
    {
        const auto insertResult = InsertInternal(key);
        NFE_ASSERT(insertResult.iterator.mElement != InvalidID, "Failed to copy element from the other hash set");
        new (mKeys + result.iterator.mElement) KeyType(key);
    }

    NFE_ASSERT(mNumKeys == oldSet.mNumKeys, "Some elements have been lost when moving from old set");
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>& HashSet<KeyType, HashFunction>::operator = (const HashSet& other)
{
    Reserve(other.Size());

    for (KeyType& key : other)
    {
        const auto insertResult = InsertInternal(key);
        NFE_ASSERT(insertResult.iterator.mElement != InvalidID, "Failed to copy element from the other hash set");
        new (mKeys + result.iterator.mElement) KeyType(key);
    }

    NFE_ASSERT(mNumKeys == oldSet.mNumKeys, "Some elements have been lost when moving from old set");

    return *this;
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>::HashSet(HashSet&& other)
{
    mKeys = other.mKeys;
    mNextElements = other.mNextElements;
    mBuckets = other.mBuckets;
    mNumKeys = other.mNumKeys;
    mHashBits = other.mHashBits;
    mFirstFreeElement = other.mFirstFreeElement;
    mFirstFreeKey = other.mFirstFreeKey;

    other.mKeys = nullptr;
    other.mNextElements = nullptr;
    other.mBuckets = nullptr;
    other.mNumKeys = 0;
    other.mHashBits = 0;
    other.mFirstFreeElement = InvalidID;
    other.mFirstFreeKey = InvalidID;
}

template<typename KeyType, typename HashFunction>
HashSet<KeyType, HashFunction>& HashSet<KeyType, HashFunction>::operator = (HashSet&& other)
{
    // TODO reuse already allocated memory
    Clear();

    mKeys = other.mKeys;
    mNextElements = other.mNextElements;
    mBuckets = other.mBuckets;
    mNumKeys = other.mNumKeys;
    mHashBits = other.mHashBits;
    mFirstFreeElement = other.mFirstFreeElement;
    mFirstFreeKey = other.mFirstFreeKey;

    other.mKeys = nullptr;
    other.mNextElements = nullptr;
    other.mBuckets = nullptr;
    other.mNumKeys = 0;
    other.mHashBits = 0;
    other.mFirstFreeElement = InvalidID;
    other.mFirstFreeKey = InvalidID;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename HashFunction>
uint32 HashSet<KeyType, HashFunction>::Size() const
{
    return mNumKeys;
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Empty() const
{
    return mNumKeys == 0;
}

template<typename KeyType, typename HashFunction>
uint32 HashSet<KeyType, HashFunction>::GetNumberOfBuckets() const
{
    // 0 bits   ->  0 buckets
    // 1 bit    ->  2 buckets
    // 2 buts   ->  4 buckets
    // etc.
    return mHashBits > 0 ? (1 << static_cast<uint32>(mHashBits)) : 0;
}

template<typename KeyType, typename HashFunction>
uint32 HashSet<KeyType, HashFunction>::GetHashMask() const
{
    return (1 << static_cast<uint32>(mHashBits)) - 1;
}

template<typename KeyType, typename HashFunction>
void HashSet<KeyType, HashFunction>::Clear()
{
    if (mKeys)
    {
        // call destructors for existing objects
        for (Iterator iter = Begin(); iter != End(); ++iter)
        {
            (*iter).~KeyType();
        }

        NFE_FREE(mKeys);
        mKeys = nullptr;
    }

    NFE_FREE(mNextElements);
    NFE_FREE(mBuckets);

    mNextElements = nullptr;
    mBuckets = nullptr;
    mNumKeys = 0;
    mHashBits = 0;
    mFirstFreeElement = InvalidID;
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::ConstIterator HashSet<KeyType, HashFunction>::Begin() const
{
    ElementID bucketID = InvalidID;
    ElementID elementID = InvalidID;
    FindFirstElement(bucketID, elementID);

    return ConstIterator(this, bucketID, elementID);
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::Iterator HashSet<KeyType, HashFunction>::Begin()
{
    ElementID bucketID = InvalidID;
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
    ElementID bucketID = InvalidID;
    ElementID elementID = InvalidID;
    FindKeyInternal(key, bucketID, elementID);

    return ConstIterator(this, bucketID, elementID);
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::Iterator HashSet<KeyType, HashFunction>::Find(const KeyType& key)
{
    ElementID bucketID = InvalidID;
    ElementID elementID = InvalidID;
    FindKeyInternal(key, bucketID, elementID);

    return Iterator(this, bucketID, elementID);
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Exists(const KeyType& key) const
{
    ElementID bucketID;
    ElementID elementID;
    return FindKeyInternal(key, bucketID, elementID);
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::InsertResult HashSet<KeyType, HashFunction>::Insert(const KeyType& key)
{
    // TODO not needed if replacing
    if (!Reserve(mNumKeys + 1))
    {
        // failed to reserve space
        return InsertResult(End());
    }

    const InsertResult result = InsertInternal(key);
    if (result.replaced)
    {
        // destroy the old object
        mKeys[result.iterator.mElement].~KeyType();
    }

    new (mKeys + result.iterator.mElement) KeyType(key);
    return result;
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::InsertResult HashSet<KeyType, HashFunction>::Insert(KeyType&& key)
{
    // TODO not needed if replacing
    if (!Reserve(mNumKeys + 1))
    {
        // failed to reserve space
        return InsertResult(End());
    }

    const auto result = InsertInternal(key);
    if (result.replaced)
    {
        // destroy the old object
        mKeys[result.iterator.mElement].~KeyType();
    }

    new (mKeys + result.iterator.mElement) KeyType(std::move(key));
    return result;
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Erase(const KeyType& key)
{
    return Erase(Find(key));
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Erase(const ConstIterator& iterator)
{
    NFE_ASSERT(iterator.mSet == this, "Trying to use iterator from another HashSet");

    if (iterator.mBucket == InvalidID)
    {
        // trying to remove not existing key
        return false;
    }

    NFE_ASSERT(iterator.mBucket < GetNumberOfBuckets(), "Trying to use invalid iterator");
    NFE_ASSERT(iterator.mElement < GetNumberOfBuckets(), "Trying to use invalid iterator");

    // find the element in the bucket
    ElementID prevElement = InvalidID;
    ElementID currentElement = mBuckets[iterator.mBucket];
    while (currentElement != iterator.mElement)
    {
        if (currentElement == InvalidID) // Element not found in the bucket
        {
            return false;
        }

        prevElement = currentElement;
        currentElement = mNextElements[currentElement];
    }

    const ElementID nextElement = mNextElements[currentElement];
    if (prevElement == InvalidID) // update list head
    {
        mBuckets[iterator.mBucket] = nextElement;
    }
    else // update internal node
    {
        mNextElements[prevElement] = nextElement;
    }

    --mNumKeys;
    mKeys[currentElement].~KeyType();
    FreeElement(currentElement);

    return true;
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Reserve(uint32 size)
{
    const uint32 currentCapacity = GetNumberOfBuckets();
    size *= RehashThresholdDenominator;
    size /= RehashThresholdNumerator;

    if (size <= currentCapacity)
    {
        // no need to rehash
        return true;
    }

    uint8 targetBits = MinimumHashBits;
    uint32 targetSize = 1 << targetBits;
    while (size > targetSize)
    {
        targetBits++;
        targetSize *= 2;
    }

    HashSet oldSet(std::move(*this));
    NFE_ASSERT(mNumKeys == 0 && mHashBits == 0, "Move constructor is broken");

    // TODO single malloc

    mKeys = reinterpret_cast<KeyType*>(NFE_MALLOC(sizeof(KeyType) * targetSize, alignof(KeyType)));
    if (!mKeys)
    {
        LOG_ERROR("Could not allocate space for hash set keys");
        return false;
    }

    // initialize bucket heads
    {
        mBuckets = reinterpret_cast<ElementID*>(NFE_MALLOC(sizeof(ElementID) * targetSize, alignof(ElementID)));
        if (!mBuckets)
        {
            LOG_ERROR("Could not allocate space for buckets heads");
            return false;
        }

        for (uint32 i = 0; i < targetSize; ++i)
        {
            mBuckets[i] = InvalidID;
        }
    }

    // initialize linked list
    {
        mNextElements = reinterpret_cast<ElementID*>(NFE_MALLOC(sizeof(ElementID) * targetSize, alignof(ElementID)));
        if (!mNextElements)
        {
            LOG_ERROR("Could not allocate space for next elements indices");
            return false;
        }

        // build free list for linked list
        // TODO build the list on the fly when inserting an elements into it
        mFirstFreeElement = 0;
        for (uint32 i = 0; i < targetSize - 1; ++i)
        {
            mNextElements[i] = i + 1;
        }
        mNextElements[targetSize - 1] = InvalidID;
    }

    mHashBits = targetBits;

    // rehash (move elements from 'oldSet' to 'this')
    for (KeyType& key : oldSet)
    {
        const auto insertResult = InsertInternal(key);
        NFE_ASSERT(insertResult.iterator.mElement != InvalidID, "Failed to move hash set key from to new buffer");
        MemoryHelpers::Move<KeyType>(mKeys + insertResult.iterator.mElement, &key);
    }

    NFE_ASSERT(mNumKeys == oldSet.mNumKeys, "Some elements have been lost when moving from old set");

    NFE_FREE(oldSet.mKeys);
    oldSet.mKeys = nullptr;
    oldSet.mNumKeys = 0;

    return true;
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::FindFirstElement(ElementID& outBucketId, ElementID& outElementId) const
{
    if (mNumKeys == 0)
    {
        return false;
    }

    // TODO this is too slow... we need linked list for allocated buckets tracking
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
bool HashSet<KeyType, HashFunction>::FindKeyInternal(const KeyType& key, ElementID& outBucketId, ElementID& outElementId) const
{
    const uint32 numberOfBuckets = GetNumberOfBuckets();
    if (numberOfBuckets == 0)
    {
        return false;
    }

    HashFunction hashFunction;

    // calculate masked hash value (equals to bucket ID)
    const uint32 hashValue = hashFunction(key) & GetHashMask();
    NFE_ASSERT(hashValue < numberOfBuckets, "Invalid hash value. This indicates hashset bug");

    // search for the key in the linked list
    ElementID currentElement = mBuckets[hashValue];
    while (currentElement != InvalidID)
    {
        NFE_ASSERT(currentElement < numberOfBuckets, "Bucket's linked list is currupted");

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

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::InsertResult HashSet<KeyType, HashFunction>::InsertInternal(const KeyType& key)
{
    // TODO split this function into two: InsertInternal, InsertOrReplaceInternal

    HashFunction hashFunction;

    const uint32 numberOfBuckets = GetNumberOfBuckets();
    const uint32 hashValue = hashFunction(key) & GetHashMask();
    NFE_ASSERT(hashValue < numberOfBuckets, "Invalid hash value. This indicates hashset bug");

    // search for the key in the linked list
    {
        ElementID currentElement = mBuckets[hashValue];
        while (currentElement != InvalidID)
        {
            NFE_ASSERT(currentElement < numberOfBuckets, "Bucket's linked list is currupted");

            if (mKeys[currentElement] == key)
            {
                return InsertResult(ConstIterator(this, hashValue, currentElement), /* replaced */ true);
            }

            currentElement = mNextElements[currentElement];
        }
    }

    const ElementID newElementID = AllocateElement();
    NFE_ASSERT(newElementID < numberOfBuckets, "Invalid element ID");

    // add next element to the bucket's linked list
    ElementID& bucketHead = mBuckets[hashValue];
    mNextElements[newElementID] = bucketHead;
    bucketHead = newElementID;

    ++mNumKeys;

    return InsertResult(ConstIterator(this, hashValue, newElementID));
}

template<typename KeyType, typename HashFunction>
typename HashSet<KeyType, HashFunction>::ElementID HashSet<KeyType, HashFunction>::AllocateElement()
{
    NFE_ASSERT(mFirstFreeElement != InvalidID, "Expected valid next free element. Linked list is corrupted");

    const ElementID result = mFirstFreeElement;
    mFirstFreeElement = mNextElements[mFirstFreeElement];
    return result;
}

template<typename KeyType, typename HashFunction>
void HashSet<KeyType, HashFunction>::FreeElement(ElementID element)
{
    NFE_ASSERT(element != InvalidID, "Expected valid element. Linked list would be corrupted");

    mNextElements[element] = mFirstFreeElement;
    mFirstFreeElement = element;
}

//////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename HashFunction>
uint32 HashSet<KeyType, HashFunction>::CalculateHashCollisions() const
{
    uint32 numCollisions = 0;

    // for each bucket
    for (uint32 i = 0; i < GetNumberOfBuckets(); ++i)
    {
        uint32 numElementsInBucket = 0;

        // go through all elements in the list
        ElementID currentElement = mBuckets[i];
        while (currentElement != InvalidID)
        {
            currentElement = mNextElements[currentElement];
            numElementsInBucket++;
        }

        if (numElementsInBucket > 1)
        {
            numCollisions += numElementsInBucket - 1;
        }
    }

    return numCollisions;
}

template<typename KeyType, typename HashFunction>
bool HashSet<KeyType, HashFunction>::Verify() const
{
    const uint32 numAllocatedElements = GetNumberOfBuckets();

    NFE_ASSERT(mNumKeys <= numAllocatedElements, "Number of inserted keys is corrupted");

    // go through all buckets' linked list and count inserted keys
    {
        uint32 numKeys = 0;
        for (uint32 i = 0; i < GetNumberOfBuckets(); ++i)
        {
            ElementID currentElement = mBuckets[i];
            while (currentElement != InvalidID)
            {
                currentElement = mNextElements[currentElement];
                numKeys++;
            }
        }

        NFE_ASSERT(numKeys == mNumKeys, "Counted elements number does not match. Hash set corruption");
    }

    // count elements in the free list
    {
        uint32 numFreeElements = 0;
        ElementID nextFree = mFirstFreeElement;
        while (nextFree != InvalidID)
        {
            nextFree = mNextElements[nextFree];
            numFreeElements++;
        }

        NFE_ASSERT(numFreeElements + mNumKeys == numAllocatedElements,
                   "Counted elements in the free list does not match. Hash set corruption");
    }

    return true;
}

} // namespace Common
} // namespace NFE
