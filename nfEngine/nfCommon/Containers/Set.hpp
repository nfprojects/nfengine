/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Set data container declaration
 */

#pragma once

#include "Comparator.hpp"

#include "../nfCommon.hpp"
#include "../Memory/DefaultAllocator.hpp"
#include "../Math/Math.hpp"
#include "../Logger.hpp"


namespace NFE {
namespace Common {

/**
 * Sorted set container (like std::set).
 * Implementation is based on AVL tree.
 */
template<typename KeyType, typename Comparator = DefaultComparator<KeyType>>
class Set final
{
public:
    static const int InvalidID;

    enum class PreviousNode
    {
        Parent,
        Left,
        Right
    };

    // generic iterator
    template<typename IterKeyType, typename IterSetType>
    class IteratorTemplate
    {
        friend class Set;
    public:
        NFE_INLINE IteratorTemplate();

        template<typename IterKeyType2, typename IterSetType2>
        NFE_INLINE bool operator == (const IteratorTemplate<IterKeyType2, IterSetType2>& other) const;

        template<typename IterKeyType2, typename IterSetType2>
        NFE_INLINE bool operator != (const IteratorTemplate<IterKeyType2, IterSetType2>& other) const;

        NFE_INLINE IterKeyType& operator*() const;
        NFE_INLINE IteratorTemplate& operator++();
        NFE_INLINE IteratorTemplate operator++(int);
    private:
        NFE_INLINE IteratorTemplate(IterSetType* set, int node);
        IterSetType* mSet;  // set we are iterating
        int mNode;          // current node ID
    };

    // iterator type specializations (const and non-const)
    using Iterator = IteratorTemplate<KeyType, Set>;
    using ConstIterator = IteratorTemplate<const KeyType, const Set>;

    // insertion information
    struct InsertResult final
    {
        // iterator to the inserted node
        ConstIterator iterator;

        // set to true if the node with given value already existed
        bool replaced;

        NFE_INLINE InsertResult(const ConstIterator& iterator, bool replaced = false)
            : iterator(iterator)
            , replaced(replaced)
        { }
    };

    // basic constructors and assignment operators

    Set();
    ~Set();
    Set(const Set& other);
    Set(Set&& other);
    Set& operator = (const Set& other);
    Set& operator = (Set&& other);

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
     * @note    This operation takes O(log n) time.
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
     * @note    This operation takes O(log n) time.
     * @return  Iterator to the found element, or iterator to the end if the element does not exist.
     */
    ConstIterator Find(const KeyType& key) const;
    Iterator Find(const KeyType& key);

    /**
     * Insert a new element to the set (by reference).
     * @note    This operation takes O(log n) time.
     * @return  Structure with insertion information.
     */
    InsertResult Insert(const KeyType& key);

    /**
     * Insert a new element to the set (by rvalue reference).
     * @note    This operation takes O(log n) time.
     * @return  Structure with insertion information.
     */
    InsertResult Insert(KeyType&& key);

    /**
     * Erase an element by value.
     * @note    This operation takes O(log n) time.
     * @return  True if the element was found and has been removed.
     */
    bool Erase(const KeyType& key);

    /**
     * Erase an element by iterator.
     * @note    This operation takes O(log n) time.
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

    /**
     * Verify internal tree structure. For debugging only.
     */
    bool Verify() const;

private:

    // internal node structure
    struct Node
    {
        union
        {
            int parent;
            int nextFree;   // used when the node is not allocated
        };

        int next[2];        // children
        int8 height;
        int8 parentIndex;
    };

    KeyType* mKeys;         //< keys buffer
    Node* mNodes;           //< nodes buffer
    int mNumElements;       //< number of inserted elements
    int mNumAllocElements;  //< size of buffers
    int mNextFreeNode;      //< head of the free list
    int mRoot;              //< root tree node

    // allocate tree node (and key)
    int AllocateNode();

    // free tree node
    void FreeNode(int node);

    // allocate node without performing actual key insert
    InsertResult InsertInternal(const KeyType& key);

    // find leftmost node (used to create 'begin' iterator)
    NFE_INLINE int LeftmostNode() const;

    // perform left tree rotation for a given node
    int RotateLeft(int x);

    // perform right tree rotation for a given node
    int RotateRight(int y);

    // rebalance node, returns new node ID after rotations
    int Rebalance(int node);

    // rebalance whole path, returns new root node
    int RebalancePath(int startNode);

    bool EraseInternal(int node);

    NFE_INLINE int8 GetNodeHeight(int node) const;
};

} // namespace Common
} // namespace NFE

// Set class definitions go here:
#include "SetImpl.hpp"
