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
 * Default hash policy is to call GetHash() global function on key element.
 */
template<typename T>
struct DefaultHashPolicy
{
    uint32 operator() (const T& in) const
    {
        return GetHash(in);
    }
};

/**
 * Hashed (unordered) set container (like std::unordered_set).
 */
template<typename KeyType, typename HashPolicy = DefaultHashPolicy<KeyType>>
class HashSet final
{
public:

    using ElementID = uint32;

    static constexpr uint32 InvalidID = static_cast<uint32>(-1);

    // minimum number of hash bits in use
    static constexpr uint8 MinimumHashBits = 4;

    // Rehash threshold in quotient form (3/4)
    // If the set usage is greater than capacity times this coefficient, rehash will be performed.
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
        ConstIterator& operator++();
        NFE_INLINE ConstIterator operator++(int);
    private:
        NFE_INLINE ConstIterator(const HashSet* set, ElementID bucket, ElementID element);
        const HashSet* mSet;    // container we are iterating
        ElementID mBucket;      // calculated bucket (masked hash)
        ElementID mElement;     // index in keys table
    };

    // iterator with read-write access
    class Iterator : public ConstIterator
    {
        friend class HashSet;
    public:
        NFE_INLINE KeyType& operator*() const;
    private:
        NFE_INLINE Iterator(const HashSet* set, ElementID bucket, ElementID element);
    };

    // insertion information
    struct InsertResult final
    {
        ConstIterator iterator; // iterator to the inserted node
        bool replaced;          // set to true if the node with given value already existed

        explicit InsertResult(const ConstIterator& iterator, bool replaced = false)
            : iterator(iterator), replaced(replaced)
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
    NFE_INLINE ConstIterator Begin() const;
    NFE_INLINE Iterator Begin();

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
    NFE_INLINE ConstIterator Find(const KeyType& key) const;
    NFE_INLINE Iterator Find(const KeyType& key);

    /**
     * Check if given key exists in the set.
     */
    NFE_INLINE bool Exists(const KeyType& key) const;

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

    // TODO InsertOrReplace

    /**
     * Erase an element by value.
     * @return  True if the element was found and has been removed.
     */
    NFE_INLINE bool Erase(const KeyType& key);

    /**
     * Erase an element by iterator.
     * @return  True if the element has been removed.
     */
    bool Erase(const ConstIterator& iterator);

    // lower-case aliases for Begin()/End(), required by C++
    NFE_INLINE ConstIterator begin() const { return Begin(); }
    NFE_INLINE ConstIterator end() const { return End(); }
    NFE_INLINE Iterator begin() { return Begin(); }
    NFE_INLINE Iterator end() { return End(); }

    /**
     * Extend set capacity. Performs rehashing.
     */
    bool Reserve(uint32 size);

    // for debugging purposes
    bool Verify() const;
    uint32 CalculateHashCollisions() const;

    NFE_INLINE uint32 GetNumberOfBuckets() const;

    /**
     * Get bit mask that needs to be applied on a hash function result.
     */
    NFE_INLINE uint32 GetHashMask() const;

private:

    // find first valid element in the set
    bool FindFirstElement(ElementID& outElementID, ElementID& outElementId) const;

    // hash set search
    bool FindKeyInternal(const KeyType& key, ElementID& outElementID, ElementID& outElementId) const;

    // allocate space for insertion
    InsertResult InsertInternal(const KeyType& key);

    // Allocate ID element in the linked list. This is O(1)
    NFE_INLINE ElementID AllocateElement();

    // Put a free ID element in the linked list. This is O(1)
    NFE_INLINE void FreeElement(ElementID element);

    // Note: this two arrays could be merged into one array of structures.
    // This should improve CPU cache performance, but also would consume much more memory if KeyType
    // requires alignment greater that 4 bytes.
    KeyType* mKeys;
    ElementID* mNextElements;       // next element IDs in the linked list

    ElementID* mBuckets;            // first element index for each bucket (linked list heads)
    ElementID mFirstFreeElement;    // first free element in the free list (for bucket data)
    ElementID mFirstFreeKey;        // first free key in the keys free list

    uint32 mNumKeys;                // number of keys inserted to the set
    uint8 mHashBits;                // number of bits of hash in use (this determines number of buckets / allocated elements)
};

} // namespace Common
} // namespace NFE


// HashSet class definitions go here:
#include "HashSetImpl.hpp"
