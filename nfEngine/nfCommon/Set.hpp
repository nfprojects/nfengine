/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Set data container
 */

#include "nfCommon.hpp"
#include "Memory/DefaultAllocator.hpp"
#include "Math/Math.hpp"
#include "Logger.hpp"


namespace NFE {
namespace Common {

// default values comparator
template<typename T>
struct DefaultComparator
{
    NFE_INLINE bool Less(const T& left, const T& right) const
    {
        return left < right;
    }

    NFE_INLINE bool Equal(const T& left, const T& right) const
    {
        return left == right;
    }
};

/**
 * Sorted set container (like std::set).
 * Implementation is based on AVL tree.
 */
template<typename KeyType, typename Comparator = DefaultComparator<KeyType>>
class Set final
{
public:
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
     * Get iterator to the beginning (smalest value).
     * @node    This operation takes O(log n) time.
     */
    ConstIterator Begin() const;
    Iterator Begin();

    /**
     * Get iterator to the end (one past the highest value).
     * @node    This operation takes O(1) time.
     */
    NFE_INLINE ConstIterator End() const;
    NFE_INLINE Iterator End();

    /**
     * Find element by value.
     * @node    This operation takes O(log n) time.
     * @return  Iterator to the found element, or iterator to the end if the element does not exist.
     */
    ConstIterator Find(const KeyType& key) const;
    Iterator Find(const KeyType& key);

    /**
     * Insert a new element to the set.
     * @node    This operation takes O(log n) time.
     * @return  Structure with insertion information.
     */
    InsertResult Insert(const KeyType& key);

    /**
     * Erase an element by value.
     * @node    This operation takes O(log n) time.
     * @return  True if the element was found and has been removed.
     */
    bool Erase(const KeyType& key);

    /**
     * Erase an element by iterator.
     * @node    This operation takes O(log n) time.
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
            int nextFree;    // used when the node is not allocated
        };

        int next[2];    // children
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

///////////////////////////////////////////////////////////////////////////////////////////////////
// Set::ConstIterator /////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename Comparator>
template<typename IterKeyType, typename IterSetType>
Set<KeyType, Comparator>::IteratorTemplate<IterKeyType, IterSetType>::IteratorTemplate()
    : mSet(nullptr)
    , mNode(-1)
{ }

template<typename KeyType, typename Comparator>
template<typename IterKeyType, typename IterSetType>
Set<KeyType, Comparator>::IteratorTemplate<IterKeyType, IterSetType>::IteratorTemplate(IterSetType* set, int node)
    : mSet(set)
    , mNode(node)
{ }

template<typename KeyType, typename Comparator>
template<typename IterKeyType, typename IterSetType>
template<typename IterKeyType2, typename IterSetType2>
bool Set<KeyType, Comparator>::IteratorTemplate<IterKeyType, IterSetType>::operator == (const IteratorTemplate<IterKeyType2, IterSetType2>& other) const
{
    return (mSet == other.mSet) && (mNode == other.mNode);
}

template<typename KeyType, typename Comparator>
template<typename IterKeyType, typename IterSetType>
template<typename IterKeyType2, typename IterSetType2>
bool Set<KeyType, Comparator>::IteratorTemplate<IterKeyType, IterSetType>::operator != (const IteratorTemplate<IterKeyType2, IterSetType2>& other) const
{
    return (mSet != other.mSet) || (mNode != other.mNode);
}

template<typename KeyType, typename Comparator>
template<typename IterKeyType, typename IterSetType>
IterKeyType& Set<KeyType, Comparator>::IteratorTemplate<IterKeyType, IterSetType>::operator*() const
{
    NFE_ASSERT(mNode != -1, "Trying to dereference 'end' iterator");
    return mSet->mKeys[mNode];
}

template<typename KeyType, typename Comparator>
template<typename IterKeyType, typename IterSetType>
typename Set<KeyType, Comparator>::IteratorTemplate<IterKeyType, IterSetType>&
    Set<KeyType, Comparator>::IteratorTemplate<IterKeyType, IterSetType>::operator++()
{
    if (mNode == -1)
    {
        // can't go beyond 'end' iterator
        return *this;
    }

    int right = mSet->mNodes[mNode].next[1];
    if (right != -1)
    {
        // go right if we can, then go leftmost to the bottom
        mNode = right;
        while (mSet->mNodes[mNode].next[0] != -1)
        {
            mNode = mSet->mNodes[mNode].next[0];
        }
    }
    else
    {
        // go up until we caome from left
        int parent;
        int8 parentIndex;
        while (parent = mSet->mNodes[mNode].parent,
               parentIndex = mSet->mNodes[mNode].parentIndex,
               (parent != -1) && (parentIndex == 1))
        {
            mNode = parent;
        }

        mNode = parent;
    }

    return *this;
}

template<typename KeyType, typename Comparator>
template<typename IterKeyType, typename IterSetType>
typename Set<KeyType, Comparator>::IteratorTemplate<IterKeyType, IterSetType>
    Set<KeyType, Comparator>::IteratorTemplate<IterKeyType, IterSetType>::operator++(int)
{
    IteratorTemplate tmp(*this);
    operator++();
    return tmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Set ////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>::Set()
    : mKeys(nullptr)
    , mNodes(nullptr)
    , mNumElements(0)
    , mNumAllocElements(0)
    , mNextFreeNode(-1)
    , mRoot(-1)
{
}

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>::~Set()
{
    Clear();
}

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>::Set(const Set& other)
{
    if (other.mNumAllocElements > 0)
    {
        size_t nodesSize = static_cast<size_t>(other.mNumAllocElements) * sizeof(Node);
        mNodes = reinterpret_cast<Node*>(NFE_MALLOC(nodesSize, 4));
        if (!mNodes)
        {
            LOG_ERROR("Can't allocate space for tree nodes");
            return;
        }

        size_t keysSize = static_cast<size_t>(other.mNumAllocElements) * sizeof(KeyType);
        mKeys = reinterpret_cast<KeyType*>(NFE_MALLOC(keysSize, 4));
        if (!mKeys)
        {
            NFE_FREE(mNodes);
            LOG_ERROR("Can't allocate space for keys");
            return;
        }

        memcpy(mNodes, other.mNodes, nodesSize);
        memcpy(mKeys, other.mKeys, keysSize);
    }

    mNumElements = other.mNumElements;
    mNumAllocElements = other.mNumAllocElements;
    mNextFreeNode = other.mNextFreeNode;
    mRoot = other.mRoot;
}

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>::Set(Set&& other)
{
    Clear();

    std::swap(mKeys, other.mKeys);
    std::swap(mNodes, other.mNodes);

    mNumElements = other.mNumElements;
    mNumAllocElements = other.mNumAllocElements;
    mNextFreeNode = other.mNextFreeNode;
    mRoot = other.mRoot;

    other.mNumElements = 0;
    other.mNumAllocElements = 0;
    other.mNextFreeNode = -1;
    other.mRoot = -1;
}

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>& Set<KeyType, Comparator>::operator = (const Set& other)
{
    if (other.mNumAllocElements > 0)
    {
        Clear();

        size_t nodesSize = static_cast<size_t>(other.mNumAllocElements) * sizeof(Node);
        mNodes = reinterpret_cast<Node*>(NFE_MALLOC(nodesSize, 4));
        if (!mNodes)
        {
            LOG_ERROR("Can't allocate space for tree nodes");
            return *this;
        }

        size_t keysSize = static_cast<size_t>(other.mNumAllocElements) * sizeof(KeyType);
        mKeys = reinterpret_cast<KeyType*>(NFE_MALLOC(keysSize, 4));
        if (!mKeys)
        {
            NFE_FREE(mNodes);
            LOG_ERROR("Can't allocate space for keys");
            return *this;
        }

        memcpy(mNodes, other.mNodes, nodesSize);
        memcpy(mKeys, other.mKeys, keysSize);
    }

    mNumElements = other.mNumElements;
    mNumAllocElements = other.mNumAllocElements;
    mNextFreeNode = other.mNextFreeNode;
    mRoot = other.mRoot;

    return *this;
}

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>& Set<KeyType, Comparator>::operator = (Set&& other)
{
    Clear();

    std::swap(mKeys, other.mKeys);
    std::swap(mNodes, other.mNodes);

    mNumElements = other.mNumElements;
    mNumAllocElements = other.mNumAllocElements;
    mNextFreeNode = other.mNextFreeNode;
    mRoot = other.mRoot;

    other.mNumElements = 0;
    other.mNumAllocElements = 0;
    other.mNextFreeNode = -1;
    other.mRoot = -1;

    return *this;
}

template<typename KeyType, typename Comparator>
uint32 Set<KeyType, Comparator>::Size() const
{
    return static_cast<uint32>(mNumElements);
}

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::Empty() const
{
    return mNumElements == 0;
}

template<typename KeyType, typename Comparator>
void Set<KeyType, Comparator>::Clear()
{
    if (mKeys)
    {
        NFE_FREE(mKeys);
        mKeys = nullptr;
    }

    if (mNodes)
    {
        NFE_FREE(mNodes);
        mNodes = nullptr;
    }

    mNumElements = 0;
    mNumAllocElements = 0;
    mNextFreeNode = -1;
    mRoot = -1;
}

template<typename KeyType, typename Comparator>
int Set<KeyType, Comparator>::LeftmostNode() const
{
    if (mRoot == -1)
    {
        // tree is empty
        return -1;
    }

    int node = mRoot;
    while (mNodes[node].next[0] != -1)
    {
        node = mNodes[node].next[0];
    }
    return node;
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::ConstIterator Set<KeyType, Comparator>::Begin() const
{
    int node = LeftmostNode();
    if (node == -1)
    {
        // tree is empty
        return End();
    }

    return ConstIterator(this, node);
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::Iterator Set<KeyType, Comparator>::Begin()
{
    int node = LeftmostNode();
    if (node == -1)
    {
        // tree is empty
        return End();
    }

    return Iterator(this, node);
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::ConstIterator Set<KeyType, Comparator>::End() const
{
    return ConstIterator(this, -1);
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::Iterator Set<KeyType, Comparator>::End()
{
    return Iterator(this, -1);
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::ConstIterator Set<KeyType, Comparator>::Find(const KeyType& key) const
{
    Comparator comparator;
    int node = mRoot;

    // classic binary search
    while (node != -1)
    {
        if (comparator.Equal(mKeys[node], key))
            return ConstIterator(this, node);

        int index = comparator.Less(mKeys[node], key) ? 1 : 0;
        node = mNodes[node].next[index];
    }

    // element not found
    return End();
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::Iterator Set<KeyType, Comparator>::Find(const KeyType& key)
{
    Comparator comparator;
    int node = mRoot;

    // classic binary search
    while (node != -1)
    {
        if (comparator.Equal(mKeys[node], key))
            return Iterator(this, node);

        int index = comparator.Less(mKeys[node], key) ? 1 : 0;
        node = mNodes[node].next[index];
    }

    // element not found
    return End();
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::InsertResult Set<KeyType, Comparator>::Insert(const KeyType& key)
{
    Comparator comparator;
    int8 index = 0;
    int node = mRoot;
    int prevNode = -1;

    // classic binary search
    while (node != -1)
    {
        // replace existing node
        if (comparator.Equal(mKeys[node], key))
        {
            mKeys[node] = key;
            return InsertResult(ConstIterator(this, node), true);
        }

        index = comparator.Less(mKeys[node], key) ? 1 : 0;
        prevNode = node;
        node = mNodes[node].next[index];
    }


    // create new node
    int newNode = AllocateNode();
    mKeys[newNode] = key;
    mNodes[newNode].next[0] = -1;
    mNodes[newNode].next[1] = -1;
    mNodes[newNode].height = 1;

    // insert new node to the tree
    mNodes[newNode].parent = prevNode;

    if (prevNode == -1)
    {
        NFE_ASSERT(mRoot == -1, "Tree is corrupted");

        mRoot = newNode;
        return InsertResult(ConstIterator(this, newNode));
    }

    mNodes[newNode].parentIndex = index;
    mNodes[prevNode].next[index] = newNode;

    // balance the tree
    mRoot = RebalancePath(prevNode);

    return InsertResult(ConstIterator(this, newNode));
}

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::Erase(const KeyType& key)
{
    Comparator comparator;
    int node = mRoot;

    // find the nodo to be removed
    while (node != -1)
    {
        if (comparator.Equal(mKeys[node], key))
            break;

        int index = comparator.Less(mKeys[node], key) ? 1 : 0;
        node = mNodes[node].next[index];
    }

    if (node == -1)
    {
        // element not found
        return false;
    }

    return Erase(ConstIterator(this, node));
}

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::Erase(const ConstIterator& iterator)
{
    if (iterator.mSet != this)
        return false;

    return EraseInternal(iterator.mNode);
}

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::Erase(const Iterator& iterator)
{
    if (iterator.mSet != this)
        return false;

    return EraseInternal(iterator.mNode);
}

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::EraseInternal(int node)
{
    if (node == -1)
        return false;

    int left = mNodes[node].next[0];
    int right = mNodes[node].next[1];
    int parent = mNodes[node].parent;
    uint8 parentIndex = mNodes[node].parentIndex;

    if (left == -1)
    {
        if (right == -1) // case 0: node to be removed has no children
        {
            if (parent != -1)
            {
                mNodes[parent].next[parentIndex] = -1;
            }
            else
            {
                NFE_ASSERT(mRoot == node, "Tree corrupted");
                mRoot = -1;
            }
        }
        else // case 1: node to be removed has only right children
        {
            mKeys[node] = mKeys[right];
            mNodes[node].next[1] = -1;
            node = right;
        }
    }
    else
    {
        if (right == -1) // case 2: node to be removed has only left children
        {
            mKeys[node] = mKeys[left];
            mNodes[node].next[0] = -1;
            node = left;
        }
        else // case 3: node to be removed has both children
        {
            // find in-order successor
            int successor = right;
            while (mNodes[successor].next[0] != -1)
            {
                successor = mNodes[successor].next[0];
            }

            // copy in-order successor data in place of deleted node
            mKeys[node] = mKeys[successor];

            right = mNodes[successor].next[1];
            if (right != -1)
            {
                // fix parent of successor's right child
                mNodes[right].parent = mNodes[successor].parent;
                mNodes[right].parentIndex = mNodes[successor].parentIndex;
            }

            // unlink successor node
            int successorParent = mNodes[successor].parent;
            int successorParentIndex = mNodes[successor].parentIndex;
            mNodes[successorParent].next[successorParentIndex] = right;

            // successor node will be removed
            node = successor;
        }
    }

    int nodeToRemove = node;

    // balance tree from the removed node
    node = mNodes[node].parent;

    // remove the node
    FreeNode(nodeToRemove);

    // balance the tree
    mRoot = RebalancePath(node);

    return true;
}


template<typename KeyType, typename Comparator>
int Set<KeyType, Comparator>::AllocateNode()
{
    if (mNextFreeNode == -1)
    {
        // calculate new buffer size
        if (mNumAllocElements == 0)
            mNumAllocElements = 16;
        else
            mNumAllocElements *= 2;

        // alloc new nodes
        Node* newNodes = reinterpret_cast<Node*>(NFE_MALLOC(mNumAllocElements * sizeof(Node), 4));
        if (!newNodes)
        {
            LOG_ERROR("Can't allocate space for tree nodes");
            return -1;
        }

        // alloc new keys
        KeyType* newKeys = reinterpret_cast<KeyType*>(NFE_MALLOC(mNumAllocElements * sizeof(KeyType), 4));
        if (!newKeys)
        {
            NFE_FREE(newNodes);
            LOG_ERROR("Can't allocate space for keys");
            return -1;
        }

        memcpy(newNodes, mNodes, mNumElements * sizeof(Node));
        memcpy(newKeys, mKeys, mNumElements * sizeof(KeyType));

        NFE_FREE(mNodes);
        NFE_FREE(mKeys);

        mNodes = newNodes;
        mKeys = newKeys;

        // initialize free list
        for (int i = mNumElements; i < mNumAllocElements - 1; ++i)
        {
            mNodes[i].nextFree = i + 1;
        }
        mNodes[mNumAllocElements - 1].nextFree = -1;
        mNextFreeNode = mNumElements;
    }

    int id = mNextFreeNode;
    mNextFreeNode = mNodes[id].nextFree;
    mNumElements++;
    return id;
}

template<typename KeyType, typename Comparator>
void Set<KeyType, Comparator>::FreeNode(int node)
{
    NFE_ASSERT(mNumElements > 0, "All nodes already released, this should never happen");

    mNodes[node].nextFree = mNextFreeNode;
    mNextFreeNode = node;
    mNumElements--;
}

template<typename KeyType, typename Comparator>
int8 Set<KeyType, Comparator>::GetNodeHeight(int node) const
{
    if (node == -1)
        return 0;

    return mNodes[node].height;
}

template<typename KeyType, typename Comparator>
int Set<KeyType, Comparator>::RebalancePath(int startNode)
{
    int node = startNode;
    while (node != -1)
    {
        mNodes[node].height = 1 + Math::Max(GetNodeHeight(mNodes[node].next[0]), GetNodeHeight(mNodes[node].next[1]));

        int parent = mNodes[node].parent;
        uint8 parentIndex = mNodes[node].parentIndex;
        int prevNode = Rebalance(node);

        if (parent == -1)
        {
            // reached the root
            return prevNode;
        }
        else
        {
            mNodes[parent].next[parentIndex] = prevNode;
            node = parent;
        }
    }

    return node;
}

template<typename KeyType, typename Comparator>
int Set<KeyType, Comparator>::Rebalance(int node)
{
    Node& nodeRef = mNodes[node];

    // update height
    int8 leftHeight = GetNodeHeight(nodeRef.next[0]);
    int8 rightHeight = GetNodeHeight(nodeRef.next[1]);
    mNodes[node].height = 1 + Math::Max(leftHeight, rightHeight);

    int balance = rightHeight - leftHeight;

    if (balance < -1) // node is left-heavy
    {
        NFE_ASSERT(balance == -2, "This should never happen");

        // TODO: instead of calculating this every time, node balance can be kept in node structure
        leftHeight = GetNodeHeight(mNodes[nodeRef.next[0]].next[0]);
        rightHeight = GetNodeHeight(mNodes[nodeRef.next[0]].next[1]);

        if (leftHeight >= rightHeight)
        {
            return RotateRight(node);
        }
        else
        {
            // TODO: full double rotation in one step
            mNodes[node].next[0] = RotateLeft(mNodes[node].next[0]);
            return RotateRight(node);
        }
    }
    else if (balance > 1) // node is right-heavy
    {
        NFE_ASSERT(balance == 2, "This should never happen");

        // TODO: instead of calculating this every time, node balance can be kept in node structure
        leftHeight = GetNodeHeight(mNodes[nodeRef.next[1]].next[0]);
        rightHeight = GetNodeHeight(mNodes[nodeRef.next[1]].next[1]);

        if (rightHeight >= leftHeight)
        {
            return RotateLeft(node);
        }
        else
        {
            // TODO: full double rotation in one step
            mNodes[node].next[1] = RotateRight(mNodes[node].next[1]);
            return RotateLeft(node);
        }
    }

    return node;
}

template<typename KeyType, typename Comparator>
int Set<KeyType, Comparator>::RotateLeft(int x)
{
    //     x                  y
    //    / \                / \
    //   T1  y     ===>     x   T3
    //      / \            / \
    //    T2  T3          T1  T2

    NFE_ASSERT(x != -1, "This indicates AVL tree bug");
    int y = mNodes[x].next[1];
    NFE_ASSERT(y != -1, "This indicates AVL tree bug");
    int T2 = mNodes[y].next[0];

    Node& nodeX = mNodes[x];
    Node& nodeY = mNodes[y];

    // perform rotation
    nodeY.next[0] = x;
    nodeX.next[1] = T2;

    // fix parent
    nodeY.parent = nodeX.parent;
    nodeY.parentIndex = nodeX.parentIndex;
    nodeX.parent = y;
    nodeX.parentIndex = 0;

    if (T2 != -1)
    {
        mNodes[T2].parent = x;
        mNodes[T2].parentIndex = 1;
    }

    // fix root parent's child
    if (nodeY.parent != -1)
    {
        NFE_ASSERT(mNodes[nodeY.parent].next[nodeY.parentIndex] == x, "Tree is corrupted");
        mNodes[nodeY.parent].next[nodeY.parentIndex] = y;
    }

    // update heights
    nodeX.height = 1 + Math::Max(GetNodeHeight(nodeX.next[0]), GetNodeHeight(nodeX.next[1]));
    nodeY.height = 1 + Math::Max(GetNodeHeight(nodeY.next[0]), GetNodeHeight(nodeY.next[1]));

    // return new root
    return y;
}

template<typename KeyType, typename Comparator>
int Set<KeyType, Comparator>::RotateRight(int y)
{
    //       y                 x
    //      / \               / \
    //     x   T3    ===>    T1  y
    //    / \                   / \
    //   T1  T2               T2  T3

    NFE_ASSERT(y != -1, "This indicates AVL tree bug");
    int x = mNodes[y].next[0];
    NFE_ASSERT(x != -1, "This indicates AVL tree bug");
    int T2 = mNodes[x].next[1];

    Node& nodeX = mNodes[x];
    Node& nodeY = mNodes[y];

    // perform rotation
    nodeX.next[1] = y;
    nodeY.next[0] = T2;

    // fix parent
    nodeX.parent = nodeY.parent;
    nodeX.parentIndex = nodeY.parentIndex;
    nodeY.parent = x;
    nodeY.parentIndex = 1;

    if (T2 != -1)
    {
        mNodes[T2].parent = y;
        mNodes[T2].parentIndex = 0;
    }

    // fix root parent's child
    if (nodeX.parent != -1)
    {
        NFE_ASSERT(mNodes[nodeX.parent].next[nodeX.parentIndex] == y, "Tree is corrupted");
        mNodes[nodeX.parent].next[nodeX.parentIndex] = x;
    }

    // update heights
    nodeY.height = 1 + Math::Max(GetNodeHeight(nodeY.next[0]), GetNodeHeight(nodeY.next[1]));
    nodeX.height = 1 + Math::Max(GetNodeHeight(nodeX.next[0]), GetNodeHeight(nodeX.next[1]));

    // return new root
    return x;
}

template<typename KeyType, typename Comparator>
void Set<KeyType, Comparator>::DebugPrint() const
{
    if (mRoot == -1)
    {
        std::cout << "<empty tree" << std::endl;
        return;
    }

    int stackDepth = 0;
    int stack[64];
    stack[stackDepth++] = mRoot;

    while (stackDepth > 0)
    {
        const int nodeID = stack[--stackDepth];
        const Node& node = mNodes[nodeID];

        std::cout << "Node #" << nodeID << std::endl;
        std::cout << "    parent: " << node.parent << " (index = " << static_cast<int>(node.parentIndex) << ")" << std::endl;
        std::cout << "    left:   " << node.next[0] << std::endl;
        std::cout << "    right:  " << node.next[1] << std::endl;
        std::cout << "    height:  " << static_cast<int>(node.height) << std::endl;
        std::cout << "    value:  " << mKeys[nodeID] << std::endl;

        if (node.next[0] != -1)
            stack[stackDepth++] = node.next[0];

        if (node.next[1] != -1)
            stack[stackDepth++] = node.next[1];
    }
}

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::Verify() const
{
    if (mRoot == -1)
    {
        if (mNumElements > 0)
        {
            LOG_ERROR("Tree corrupted");
            return false;
        }

        return true;
    }

    Comparator comparator;

    // max AVL tree depth is equal approximateley to 1.44 * log2(N + 2) - 0.328
    // for N = 2^31 it gives 45
    const int maxTreeDepth = 45;
    int stackDepth = 0;
    int stack[maxTreeDepth];

    int nodeCounter = 0;

    stack[stackDepth++] = mRoot;
    while (stackDepth > 0)
    {
        nodeCounter++;
        int node = stack[--stackDepth];
        int left = mNodes[node].next[0];
        int right = mNodes[node].next[1];
        int leftHeight = 0, rightHeight = 0;

        if (left != -1)
        {
            stack[stackDepth++] = left;
            leftHeight = mNodes[left].height;

            if (mNodes[left].parent != node || mNodes[left].parentIndex != 0)
            {
                LOG_ERROR("Parent information corrupted");
                return false;
            }

            if (comparator.Less(mKeys[node], mKeys[left]))
            {
                LOG_ERROR("Tree nodes conditions not fullfiled");
                return false;
            }
        }

        if (right != -1)
        {
            stack[stackDepth++] = right;
            rightHeight = mNodes[right].height;

            if (mNodes[right].parent != node || mNodes[right].parentIndex != 1)
            {
                LOG_ERROR("Parent information corrupted");
                return false;
            }

            if (comparator.Less(mKeys[right], mKeys[node]))
            {
                LOG_ERROR("Tree nodes conditions not fullfiled");
                return false;
            }
        }

        if (mNodes[node].height != (1 + Math::Max(leftHeight, rightHeight)))
        {
            LOG_ERROR("Node height is invalid");
            return false;
        }

        if (stackDepth > maxTreeDepth)
        {
            LOG_ERROR("Tree is too deep");
            return false;
        }
    }

    if (mNumElements != nodeCounter)
    {
        LOG_ERROR("Tree corrupted");
        return false;
    }

    return true;
}

} // namespace Common
} // namespace NFE
