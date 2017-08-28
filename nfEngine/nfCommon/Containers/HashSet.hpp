/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  HashSet data container declaration
 */

#pragma once

#include "../nfCommon.hpp"
#include "Hash.hpp"


namespace NFE {
namespace Common {

/**
 * Hashed (unordered) set container (like std::unordered_set).
 */
template<typename KeyType, typename HashFunction = Hash<KeyType, uint32>>
class HashSet final
{
public:

    using BucketID = uint32;
    using ElementID = uint32;

    static constexpr uint32 InvalidID = static_cast<uint32>(-1);

    // minimum number of allocated elements in the set
    static constexpr uint32 InitialSize = 16;
    
    // rehash threshold in quotient form (3/4)
    static constexpr uint32 RehashThresholdNumerator = 3;
    static constexpr uint32 RehashThresholdDenominator = 4;

    // iterator with read-only access
    class ConstIterator
    {
        friend class HashSet;
    public:
        NFE_INLINE ConstIterator();
        NFE_INLINE bool operator == (const ConstIterator& other) const;
        NFE_INLINE bool operator != (const ConstIterator& other) const;
        NFE_INLINE const KeyType& operator*() const;
        NFE_INLINE ConstIterator& operator++();
        NFE_INLINE ConstIterator operator++(int);
    private:
        NFE_INLINE ConstIterator(const HashSet* set, BucketID bucket, ElementID element);
        const HashSet* mSet;    // container we are iterating
        BucketID mBucket;
        ElementID mElement;
    };

    // iterator with read-write access
    class Iterator : public ConstIterator
    {
        friend class HashSet;
    public:
        NFE_INLINE KeyType& operator*() const;
    private:
        NFE_INLINE Iterator(const HashSet* set, BucketID bucket, ElementID element);
    };

    // insertion information
    struct InsertResult final
    {
        // iterator to the inserted node
        ConstIterator iterator;

        // set to true if the node with given value already existed
        bool replaced;

        explicit InsertResult(const ConstIterator& iterator, bool replaced = false)
            : iterator(iterator)
            , replaced(replaced)
        { }
    };

    // basic constructors and assignment operators
    HashSet();
    ~HashSet();
    HashSet(const HashSet& other);
    HashSet(HashSet&& other);
    HashSet& operator = (const HashSet& other);
    HashSet& operator = (HashSet&& other);

    /**
     * Get number of inserted values.
     */
    NFE_INLINE uint32 Size() const;

    /**
     * Check if the set is empty.
     */
    NFE_INLINE bool Empty() const;

    /**
     * Remove all the values and free allocated memory.
     * TODO: clear without memory deallocation
     */
    void Clear();

    /**
     * Get iterator to the beginning (smallest value).
     */
    ConstIterator Begin() const;
    Iterator Begin();

    /**
     * Get iterator to the end (one past the highest value).
     * @note    This operation takes O(1) time.
     */
    NFE_INLINE ConstIterator End() const;
    NFE_INLINE Iterator End();

    /**
     * Find element by value.
     * @return  Iterator to the found element, or iterator to the end if the element does not exist.
     */
    ConstIterator Find(const KeyType& key) const;
    Iterator Find(const KeyType& key);

    /**
     * Insert a new element to the set (by reference).
     * @return  Structure with insertion information.
     */
    InsertResult Insert(const KeyType& key);

    /**
     * Insert a new element to the set (by rvalue reference).
     * @return  Structure with insertion information.
     */
    InsertResult Insert(KeyType&& key);

    /**
     * Erase an element by value.
     * @return  True if the element was found and has been removed.
     */
    bool Erase(const KeyType& key);

    /**
     * Erase an element by iterator.
     * @return  True if the element has been removed.
     */
    bool Erase(const ConstIterator& iterator);
    bool Erase(const Iterator& iterator);

    // lower-case aliases for Begin()/End(), required by C++
    NFE_INLINE ConstIterator begin() const { return Begin(); }
    NFE_INLINE ConstIterator end() const { return End(); }
    NFE_INLINE Iterator begin() { return Begin(); }
    NFE_INLINE Iterator end() { return End(); }

    // for debugging purposes
    void DebugPrint() const;
    bool Verify() const;

    NFE_INLINE uint32 GetNumberOfBuckets() const;

    /**
     * Get bit mask that needs to be applied on a hash function result.
     */
    NFE_INLINE uint32 GetHashMask() const;

private:

    // find first valid element in the set
    bool FindFirstElement(BucketID& outBucketId, ElementID& outElementId) const;

    // hash set search
    bool FindKeyInternal(const KeyType& key, BucketID& outBucketId, ElementID& outElementId) const;

    // Note: this two arrays could be merged into one array of structures.
    // This should improve CPU cache performance, but also would consume much more memory if KeyType
    // requires alignment greater that 4 bytes.
    KeyType* mKeys;
    ElementID* mNextElements; // next element IDs in the linked list

    // first element index for each bucket
    ElementID* mBuckets;
 
    uint32 mNumElements;        // number of elements inserted to the set
    uint32 mAllocatedSize;      // number of allocated elements
    uint8 mHashBits;           // number of bits of hash in use (this determines number of buckets)
};

} // namespace Common
} // namespace NFE


// HashSet class definitions go here:
#include "HashSetImpl.hpp"
