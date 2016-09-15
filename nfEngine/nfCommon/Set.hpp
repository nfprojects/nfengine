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
    NFE_INLINE int operator()(const T& left, const T& right) const
    {
        if (left < right)
            return 1;
        else if (left > right)
            return -1;
        else
            return 0;
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

    // interator for read-only access
    class ConstIterator
    {
        friend class Set;
    public:
        NFE_INLINE ConstIterator(const Set* set, int node);
        NFE_INLINE bool operator == (const ConstIterator& other) const;
        NFE_INLINE bool operator != (const ConstIterator& other) const;
        NFE_INLINE const KeyType& operator*() const;
        NFE_INLINE ConstIterator& operator++();
        NFE_INLINE ConstIterator operator++(int);
    private:
        const Set* mSet;        // set we are iterating
        int mNode;              // current node ID
    };

    /*
    // interator for read-write access
    class Iterator final : public ConstIterator
    {
    public:
        NFE_INLINE Iterator(Set* set, int node);
        NFE_INLINE KeyType& operator*();
    };
    */

    // insertion information
    struct InsertResult final
    {
        ConstIterator iterator;
        NFE_INLINE InsertResult(const ConstIterator& iterator) : iterator(iterator) { }
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
     * Remove all the values.
     */
    void Clear() const;

    /**
     * Get const iterator to the beginning (smalest value).
     * @node    This operation takes O(log n) time.
     */
    ConstIterator Begin() const;

    /**
     * Get const iterator to the end (one past the highest value).
     * @node    This operation takes O(1) time.
     */
    NFE_INLINE ConstIterator End() const;

    /**
     * Find element by value.
     * @node    This operation takes O(log n) time.
     * @return  Iterator to the found element, or iterator to the end if the element does not exist.
     */
    ConstIterator Find(const KeyType& key) const;

    /**
     * Insert a new element to the set.
     * @node    This operation takes O(log n) time.
     * @return  Structure with isertion information.
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

    // lower-case aliases for Begin()/End(), required by C++
    ConstIterator begin() const { return Begin(); }
    ConstIterator end() const { return End(); }

    // for debugging purposes
    void DebugPrint() const;

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
        int height;
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

    // insert tree node
    int InsertInternal(int node, const KeyType& key);

    // delete tree node
    int DeleteInternal(int node, const KeyType& key);

    // perform left tree rotation for a given node
    int RotateLeft(int x);

    // perform right tree rotation for a given node
    int RotateRight(int y);

    // rebalance node, returns new node ID after rotations
    int Rebalance(int node);

    NFE_INLINE int GetNodeHeight(int node) const;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Set::ConstIterator /////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>::ConstIterator::ConstIterator(const Set* set, int node)
    : mSet(set)
    , mNode(node)
{ }

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::ConstIterator::operator == (const ConstIterator& other) const
{
    return (mSet == other.mSet) && (mNode == other.mNode);
}

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::ConstIterator::operator != (const ConstIterator& other) const
{
    return (mSet != other.mSet) || (mNode != other.mNode);
}

template<typename KeyType, typename Comparator>
const KeyType& Set<KeyType, Comparator>::ConstIterator::operator*() const
{
    NFE_ASSERT(mNode != -1, "Trying to dereference 'end' iterator");
    return mSet->mKeys[mNode];
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::ConstIterator& Set<KeyType, Comparator>::ConstIterator::operator++()
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
        while (parent = mSet->mNodes[mNode].parent,
               (parent != -1) && (mNode == mSet->mNodes[parent].next[1]))
        {
            mNode = parent;
        }

        mNode = parent;
    }

    return *this;
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::ConstIterator Set<KeyType, Comparator>::ConstIterator::operator++(int)
{
    ConstIterator tmp(*this);
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
}

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>::Set(const Set& other)
{
    // TODO
}

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>::Set(Set&& other)
{
    // TODO
}

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>& Set<KeyType, Comparator>::operator = (const Set& other)
{
    // TODO
}

template<typename KeyType, typename Comparator>
Set<KeyType, Comparator>& Set<KeyType, Comparator>::operator = (Set&& other)
{
    // TODO
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
void Set<KeyType, Comparator>::Clear() const
{
    // TODO
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
typename Set<KeyType, Comparator>::ConstIterator Set<KeyType, Comparator>::End() const
{
    return ConstIterator(this, -1);
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::ConstIterator Set<KeyType, Comparator>::Find(const KeyType& key) const
{
    int node = mRoot;

    // classic binary search
    while (node != -1)
    {
        Comparator comparator;
        int result = comparator(mKeys[node], key);

        if (result < 0)
            node = mNodes[node].next[0];
        else if (result > 0)
            node = mNodes[node].next[1];
        else
            return ConstIterator(this, node);
    }

    // element not found
    return End();
}

template<typename KeyType, typename Comparator>
typename Set<KeyType, Comparator>::InsertResult Set<KeyType, Comparator>::Insert(const KeyType& key)
{
    // TODO: non-recursive insertion

    int newRoot = InsertInternal(mRoot, key);
    mRoot = newRoot;
    mNodes[mRoot].parent = -1;

    return InsertResult(ConstIterator(this, newRoot));
}

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::Erase(const KeyType& key)
{
    if (mRoot == -1)
        return false;

    mRoot = DeleteInternal(mRoot, key);
    return mRoot != -1;
}

template<typename KeyType, typename Comparator>
bool Set<KeyType, Comparator>::Erase(const ConstIterator& iterator)
{
    if (iterator.mNode == -1)
        return false;

    return Erase(mKeys[iterator.mNode]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

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
int Set<KeyType, Comparator>::GetNodeHeight(int node) const
{
    if (node == -1)
        return 0;

    return mNodes[node].height;
}

template<typename KeyType, typename Comparator>
int Set<KeyType, Comparator>::InsertInternal(int node, const KeyType& key)
{
    if (node == -1)
    {
        int newNode = AllocateNode();
        mKeys[newNode] = key;
        mNodes[newNode].parent = -1;
        mNodes[newNode].next[0] = -1;
        mNodes[newNode].next[1] = -1;
        mNodes[newNode].height = 1;
        return newNode;
    }

    Comparator comparator;

    // insert new node
    int comparisonResult = comparator(mKeys[node], key);
    if (comparisonResult < 0)
    {
        int newNode = InsertInternal(mNodes[node].next[0], key);
        mNodes[node].next[0] = newNode;
        mNodes[newNode].parent = node;
    }
    else if (comparisonResult > 0)
    {
        int newNode = InsertInternal(mNodes[node].next[1], key);
        mNodes[node].next[1] = newNode;
        mNodes[newNode].parent = node;
    }
    else
    {
        return -1;
    }

    return Rebalance(node);
}

template<typename KeyType, typename Comparator>
int Set<KeyType, Comparator>::DeleteInternal(int node, const KeyType& key)
{
    UNUSED(node);
    UNUSED(key);
    return -1;

    /*
    if (node == -1)
        return -1;

    Comparator comparator;

    int comparisonResult = comparator(mKeys[node], key);
    if (comparisonResult < 0)
    {
        mNodes[node].next[0] = DeleteInternal(mNodes[node].next[0], key);
        mNodes[mNodes[node].next[0]].parent = node;
    }
    else if (comparisonResult > 0)
    {
        mNodes[node].next[1] = DeleteInternal(mNodes[node].next[1], key);
        mNodes[mNodes[node].next[1]].parent = node;
    }
    else // found node to remove
    {
        if (mNodes[node].next[1] == -1 || mNodes[node].next[0] == -1) // node with one or zero children
        {
            int temp = (mNodes[node].next[0] != -1) ? mNodes[node].next[0] : mNodes[node].next[1];

            if (temp == -1)
            {
                temp = node;
                node = -1;
            }

            FreeNode(temp);
        }

        if (node == -1)
            return -1;
    }

    return Rebalance(node);
    */
}

template<typename KeyType, typename Comparator>
int Set<KeyType, Comparator>::Rebalance(int node)
{
    Node& nodeRef = mNodes[node];

    // update height
    int leftHeight = GetNodeHeight(nodeRef.next[0]);
    int rightHeight = GetNodeHeight(nodeRef.next[1]);
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
    nodeX.parent = y;
    if (T2 != -1)
        mNodes[T2].parent = x;

    // fix root parent's child
    if (nodeY.parent != -1)
    {
        if (mNodes[nodeY.parent].next[1] == x)
            mNodes[nodeY.parent].next[1] = y;
        else
            mNodes[nodeY.parent].next[0] = y;
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
    nodeY.parent = x;
    if (T2 != -1)
        mNodes[T2].parent = y;

    // fix root parent's child
    if (nodeX.parent != -1)
    {
        if (mNodes[nodeX.parent].next[1] == y)
            mNodes[nodeX.parent].next[1] = x;
        else
            mNodes[nodeX.parent].next[0] = x;
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
        std::cout << "    parent: " << node.parent << std::endl;
        std::cout << "    left:   " << node.next[0] << std::endl;
        std::cout << "    right:  " << node.next[1] << std::endl;
        std::cout << "    height:  " << node.height << std::endl;
        std::cout << "    value:  " << mKeys[nodeID] << std::endl;

        if (node.next[0] != -1)
            stack[stackDepth++] = node.next[0];

        if (node.next[1] != -1)
            stack[stackDepth++] = node.next[1];
    }
}

} // namespace Common
} // namespace NFE
