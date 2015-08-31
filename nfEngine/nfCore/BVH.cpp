/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Bounding Volume Hierarchy declaration.
 */

#include "PCH.hpp"
#include "BVH.hpp"
#include "../nfCommon/Math/Box.hpp"
#include "../nfCommon/Math/Frustum.hpp"
#include "../nfCommon/Math/Geometry.hpp"

namespace NFE {
namespace Util {

using namespace Math;

#define X_BVH_STACK_SIZE 128
#define X_BVH_INITIAL_CAPACITY 128
#define X_BVH_NULL_NODE 0xFFFFFFFF

BVHNode::BVHNode()
{
    parent = 0;
    userData = 0;
    child[0] = child[1] = 0;
}

void* BVHNode::operator new(size_t size)
{
    return _aligned_malloc(size, 16);
}

void BVHNode::operator delete(void* ptr)
{
    _aligned_free(ptr);
}

bool BVHNode::IsLeaf() const
{
    return (child[0] == X_BVH_NULL_NODE);
}


BVH::BVH()
{
    mNodes = nullptr;
    Clear();
}

BVH::~BVH()
{
    if (mNodes)
    {
        _aligned_free(mNodes);
        mNodes = 0;
    }
}

void BVH::Clear()
{
    mRoot = X_BVH_NULL_NODE;
    mLeavesNum = 0;

    mNodesCapacity = X_BVH_INITIAL_CAPACITY;
    mNodesNum = 0;
    mNodes = (BVHNode*)_aligned_realloc(mNodes, mNodesCapacity * sizeof(BVHNode), 16);

    // init free list
    for (uint32 i = 0; i < mNodesCapacity - 1; i++)
        mNodes[i].next = i + 1;
    mNodes[mNodesCapacity - 1].next = X_BVH_NULL_NODE;
    mFreeNode = 0;

    mRebalancePath = 0;
}

uint32 BVH::GetSize() const
{
    return mLeavesNum;
}

// alocate space for a new node from the free list
uint32 BVH::AllocNode()
{
    // expand free list
    if (mFreeNode == X_BVH_NULL_NODE)
    {
        mNodesCapacity *= 2;
        mNodes = (BVHNode*)_aligned_realloc(mNodes, mNodesCapacity * sizeof(BVHNode), 16);

        for (uint32 i = mNodesNum; i < mNodesCapacity - 1; i++)
            mNodes[i].next = i + 1;
        mNodes[mNodesCapacity - 1].next = X_BVH_NULL_NODE;
        mFreeNode = mNodesNum;
    }

    int id = mFreeNode;
    mFreeNode = mNodes[mFreeNode].next;
    mNodes[id].parent = X_BVH_NULL_NODE;
    mNodes[id].child[0] = X_BVH_NULL_NODE;
    mNodes[id].child[1] = X_BVH_NULL_NODE;

    mNodesNum++;
    return id;
}

// mark node as free
void BVH::FreeNode(uint32 nodeID)
{
    NFE_ASSERT(nodeID < mNodesCapacity, L"Invalid nodeID");
    NFE_ASSERT(0 < mNodesNum, L"Tree is empty");

    mNodes[nodeID].next = mFreeNode;
    mFreeNode = nodeID;
    mNodesNum--;

    // TODO: if (mNodesNum < hysteresis_factor * mNodesCapacity) then rearrange and reallocate list

}

uint32 BVH::Insert(const Box& aabb, void* pUserData)
{
    uint32 leaf = AllocNode();
    mNodes[leaf].AABB = Box(aabb, aabb); //make sure that (min <= max)
    mNodes[leaf].userData = pUserData;
    InsertLeaf(leaf);

    Rebalance(1);// + (mNodesNum>>5));

    return leaf;
}

bool BVH::Remove(uint32 nodeID)
{
    if (nodeID >= mNodesCapacity || !mNodes[nodeID].IsLeaf())
        return false;

    RemoveLeaf(nodeID);
    FreeNode(nodeID);
    return true;
}

bool BVH::Move(uint32 nodeID, const Box& aabb)
{
    if (nodeID >= mNodesCapacity || !mNodes[nodeID].IsLeaf())
        return false;

    // TODO:
    // if (mnodes[proxyId].aabb.Contains(aabb)) return;

    RemoveLeaf(nodeID);
    mNodes[nodeID].AABB = aabb;
    InsertLeaf(nodeID);

    return true;
}

// Turns allocated node into leaf
void BVH::InsertLeaf(uint32 leaf)
{
    mLeavesNum++;

    // empty tree => make node as root
    if (mRoot == X_BVH_NULL_NODE)
    {
        mRoot = leaf;
        mNodes[mRoot].parent = X_BVH_NULL_NODE;
        return;
    }

    uint32 sibling = mRoot;
    if (!mNodes[sibling].IsLeaf())
    {
        do
        {
            const Box childBoxA = mNodes[mNodes[sibling].child[0]].AABB;
            const Box childBoxB = mNodes[mNodes[sibling].child[1]].AABB;

            const Box boxA = Box(mNodes[leaf].AABB, childBoxA);
            const Box boxB = Box(mNodes[leaf].AABB, childBoxB);

            const float volumeA = boxA.Volume() - childBoxA.Volume();
            const float volumeB = boxB.Volume() - childBoxB.Volume();

            if (volumeA > volumeB)
                sibling = mNodes[sibling].child[1];
            else
                sibling = mNodes[sibling].child[0];

        }
        while (!mNodes[sibling].IsLeaf());
    }


    //create node for siblings
    uint32 node2 = AllocNode();
    uint32 node1 = mNodes[sibling].parent;
    mNodes[node2].parent = node1;
    mNodes[node2].AABB = Box(mNodes[leaf].AABB, mNodes[sibling].AABB);

    if (node1 != X_BVH_NULL_NODE)
    {
        if (mNodes[mNodes[sibling].parent].child[0] == sibling)
            mNodes[node1].child[0] = node2;
        else
            mNodes[node1].child[1] = node2;

        mNodes[node2].child[0] = sibling;
        mNodes[node2].child[1] = leaf;
        mNodes[sibling].parent = node2;
        mNodes[leaf].parent = node2;

        // correct AABB up to the root
        do
        {
            mNodes[node1].AABB = Box(mNodes[mNodes[node1].child[0]].AABB,
                                     mNodes[mNodes[node1].child[1]].AABB);
            node2 = node1;
            node1 = mNodes[node1].parent;
        }
        while (node1 != X_BVH_NULL_NODE);
    }
    else
    {
        mNodes[node2].child[0] = sibling;
        mNodes[node2].child[1] = leaf;
        mNodes[sibling].parent = node2;
        mNodes[leaf].parent = node2;
        mRoot = node2;
    }
}

void BVH::RemoveLeaf(uint32 leaf)
{
    if (mRoot == leaf)
    {
        mRoot = X_BVH_NULL_NODE;
        return;
    }

    uint32 node2 = mNodes[leaf].parent;
    uint32 node1 = mNodes[node2].parent;
    uint32 sibling;
    if (mNodes[node2].child[0] == leaf)
        sibling = mNodes[node2].child[1];
    else
        sibling = mNodes[node2].child[0];

    if (node1 != X_BVH_NULL_NODE)
    {
        // Destroy node2 and connect node1 to sibling.
        if (mNodes[node1].child[0] == node2)
            mNodes[node1].child[0] = sibling;
        else
            mNodes[node1].child[1] = sibling;

        mNodes[sibling].parent = node1;
        FreeNode(node2);

        // Adjust ancestor bounds.
        while (node1 != X_BVH_NULL_NODE)
        {
            Box oldAABB = mNodes[node1].AABB;
            mNodes[node1].AABB = Box(mNodes[mNodes[node1].child[0]].AABB,
                                     mNodes[mNodes[node1].child[1]].AABB);

            //if (oldAABB.Contains(mNodes[node1].aabb))
            //  break;

            node1 = mNodes[node1].parent;
        }
    }
    else
    {
        mRoot = sibling;
        mNodes[sibling].parent = X_BVH_NULL_NODE;
        FreeNode(node2);
    }
}

void BVH::Rebalance(uint32 iterations)
{
    if (mRoot == X_BVH_NULL_NODE) return;

    for (uint32 i = 0; i < iterations; i++)
    {
        int32 node = mRoot;

        uint32 bit = 0;
        while (!mNodes[node].IsLeaf())
        {
            uint32* pChildren = mNodes[node].child;
            node = pChildren[(mRebalancePath >> bit) & 1];
            bit = (bit + 1) & (8 * sizeof(uint32) - 1);
        }
        mRebalancePath *= 1103515245;
        mRebalancePath += 12345;

        RemoveLeaf(node);
        InsertLeaf(node);
    }
}

uint32 BVH::CalculateHeight(uint32 nodeID) const
{
    if (nodeID == X_BVH_NULL_NODE) return 0;
    NFE_ASSERT(nodeID < mNodesCapacity, L"Invalid nodeID");

    uint32 leftHeight = CalculateHeight(mNodes[nodeID].child[0]);
    uint32 rightHeight = CalculateHeight(mNodes[nodeID].child[1]);
    return Max(leftHeight, rightHeight) + 1;
}

uint32 BVH::CalculateHeight() const
{
    return CalculateHeight(mRoot);
}

void BVH::GetNodeStats(uint32 nodeID, BVHStats* pStats) const
{
    if (nodeID == X_BVH_NULL_NODE) return;
    NFE_ASSERT(nodeID < mNodesCapacity, L"Invalid nodeID");

    pStats->totalArea += (double)mNodes[nodeID].AABB.SurfaceArea();
    pStats->totatlVolume += (double)mNodes[nodeID].AABB.Volume();

    GetNodeStats(mNodes[nodeID].child[0], pStats);
    GetNodeStats(mNodes[nodeID].child[1], pStats);
}

void BVH::GetStats(BVHStats* pStats) const
{
    pStats->height = CalculateHeight();
    pStats->nodesNum = mNodesNum;
    pStats->leavesNum = mLeavesNum;

    pStats->totalArea = 0.0;
    pStats->totatlVolume = 0.0;
    GetNodeStats(mRoot, pStats);
}

// get root node ID
uint32 BVH::GetRootId() const
{
    return mRoot;
}

// get node object
BVHNode* BVH::GetNodeById(uint32 id) const
{
    // TODO: range check
    return mNodes + id;
}

void BVH::QueryAll(uint32 node, BVHQueryCallback pCallback, void* pUserData) const
{
    int stackDepth = 0;
    uint32 pStack[X_BVH_STACK_SIZE];
    pStack[stackDepth++] = node;

    while (stackDepth > 0)
    {
        BVHNode* pNode = mNodes + pStack[--stackDepth];

        if (pNode->IsLeaf())
        {
            pCallback(pNode->userData, pUserData);
        }
        else if (stackDepth + 2 <= X_BVH_STACK_SIZE)
        {
            pStack[stackDepth++] = pNode->child[0];
            pStack[stackDepth++] = pNode->child[1];
        }
    }
}

template <>
CORE_API void BVH::Query(const Box& shape, BVHQueryCallback pCallback, void* pUserData) const
{
    if (mRoot == X_BVH_NULL_NODE)
        return;

    int stackDepth = 0;
    uint32 pStack[X_BVH_STACK_SIZE];
    pStack[stackDepth++] = mRoot;

    while (stackDepth > 0)
    {
        BVHNode* pNode = mNodes + pStack[--stackDepth];

        if (Intersect(pNode->AABB, shape))
        {
            if (pNode->IsLeaf())
                pCallback(pNode->userData, pUserData);
            else if (stackDepth + 2 <= X_BVH_STACK_SIZE)
            {
                pStack[stackDepth++] = pNode->child[0];
                pStack[stackDepth++] = pNode->child[1];
            }
        }
    }
}

template <>
CORE_API void BVH::Query(const Frustum& shape, BVHQueryCallback pCallback, void* pUserData) const
{
    if (mRoot == X_BVH_NULL_NODE)
        return;

    int stackDepth = 0;
    uint32 pStack[X_BVH_STACK_SIZE];
    pStack[stackDepth++] = mRoot;

    while (stackDepth > 0)
    {
        uint32 nodeID = pStack[--stackDepth];
        const BVHNode* pNode = mNodes + nodeID;

        IntersectionResult result = IntersectEx(pNode->AABB, shape);
        if (result == IntersectionResult::Inside)
        {
            QueryAll(nodeID, pCallback, pUserData);
            continue;
        }
        else if (result == IntersectionResult::Intersect)
        {
            if (pNode->IsLeaf())
            {
                pCallback(pNode->userData, pUserData);
            }
            else if (stackDepth + 2 <= X_BVH_STACK_SIZE)
            {
                pStack[stackDepth++] = pNode->child[0];
                pStack[stackDepth++] = pNode->child[1];
            }
        }
    }
}

} // namespace Util
} // namespace NFE
