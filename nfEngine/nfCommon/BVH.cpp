/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Bounding Volume Hierarchy declaration.
 */

#include "PCH.hpp"
#include "BVH.hpp"
#include "Math/Box.hpp"
#include "Math/Frustum.hpp"
#include "Math/Geometry.hpp"

namespace NFE {
namespace Common {

using namespace Math;

#define NFE_BVH_STACK_SIZE 128
#define NFE_BVH_INITIAL_CAPACITY 128

BVHNode::BVHNode()
{
    parent = 0;
    userData = 0;
    child[0] = child[1] = 0;
}


BVH::BVH()
{
    Clear();
}

void BVH::Clear()
{
    mRoot = NFE_BVH_NULL_NODE;
    mLeavesNum = 0;

    mNodesCapacity = NFE_BVH_INITIAL_CAPACITY;
    mNodesNum = 0;
    mNodes.resize(mNodesCapacity);

    // init free list
    for (uint32 i = 0; i < mNodesCapacity - 1; i++)
        mNodes[i].next = i + 1;
    mNodes[mNodesCapacity - 1].next = NFE_BVH_NULL_NODE;
    mFreeNode = 0;

    mRebalancePath = 0;
}

// alocate space for a new node from the free list
uint32 BVH::AllocNode()
{
    // expand free list
    if (mFreeNode == NFE_BVH_NULL_NODE)
    {
        mNodesCapacity *= 2;
        mNodes.resize(mNodesCapacity);

        for (uint32 i = mNodesNum; i < mNodesCapacity - 1; i++)
            mNodes[i].next = i + 1;
        mNodes[mNodesCapacity - 1].next = NFE_BVH_NULL_NODE;
        mFreeNode = mNodesNum;
    }

    int id = mFreeNode;
    mFreeNode = mNodes[mFreeNode].next;
    mNodes[id].parent = NFE_BVH_NULL_NODE;
    mNodes[id].child[0] = NFE_BVH_NULL_NODE;
    mNodes[id].child[1] = NFE_BVH_NULL_NODE;

    mNodesNum++;
    return id;
}

// mark node as free
void BVH::FreeNode(uint32 nodeID)
{
    mNodes[nodeID].next = mFreeNode;
    mNodes[nodeID].child[0] = NFE_BVH_NULL_NODE;
    mFreeNode = nodeID;
    mNodesNum--;
}

uint32 BVH::Insert(const Box& aabb, void* userData)
{
    uint32 leaf = AllocNode();
    mNodes[leaf].AABB = Box(aabb, aabb); //make sure that (min <= max)
    mNodes[leaf].userData = userData;
    mNodes[leaf].child[0] = NFE_BVH_LEAF_NODE;
    InsertLeaf(leaf);

    Rebalance(1);// + (mNodesNum>>5));

    mLeavesNum++;
    return leaf;
}

bool BVH::Remove(uint32 nodeID)
{
    if (nodeID >= mNodesCapacity || !mNodes[nodeID].IsLeaf())
        return false;

    RemoveLeaf(nodeID);
    FreeNode(nodeID);
    mLeavesNum--;
    return true;
}

bool BVH::Move(uint32 nodeID, const Box& aabb)
{
    if (nodeID >= mNodesCapacity || !mNodes[nodeID].IsLeaf())
        return false;

    RemoveLeaf(nodeID);
    mNodes[nodeID].AABB = aabb;
    InsertLeaf(nodeID);
    return true;
}

// Turns allocated node into leaf
void BVH::InsertLeaf(uint32 leaf)
{
    // empty tree => make node as root
    if (mRoot == NFE_BVH_NULL_NODE)
    {
        mRoot = leaf;
        mNodes[mRoot].parent = NFE_BVH_NULL_NODE;
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

    if (node1 != NFE_BVH_NULL_NODE)
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
        while (node1 != NFE_BVH_NULL_NODE);
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
        mRoot = NFE_BVH_NULL_NODE;
        return;
    }

    uint32 node2 = mNodes[leaf].parent;
    uint32 node1 = mNodes[node2].parent;
    uint32 sibling;
    if (mNodes[node2].child[0] == leaf)
        sibling = mNodes[node2].child[1];
    else
        sibling = mNodes[node2].child[0];

    if (node1 != NFE_BVH_NULL_NODE)
    {
        // Destroy node2 and connect node1 to sibling.
        if (mNodes[node1].child[0] == node2)
            mNodes[node1].child[0] = sibling;
        else
            mNodes[node1].child[1] = sibling;

        mNodes[sibling].parent = node1;
        FreeNode(node2);

        // Adjust ancestor bounds.
        while (node1 != NFE_BVH_NULL_NODE)
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
        mNodes[sibling].parent = NFE_BVH_NULL_NODE;
        FreeNode(node2);
    }
}

// TODO: replace this with tree rotations
void BVH::Rebalance(uint32 iterations)
{
    if (mRoot == NFE_BVH_NULL_NODE)
        return;

    for (uint32 i = 0; i < iterations; i++)
    {
        int32 node = mRoot;

        uint32 bit = 0;
        while (!mNodes[node].IsLeaf())
        {
            uint32* children = mNodes[node].child;
            node = children[(mRebalancePath >> bit) & 1];
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
    if (nodeID == NFE_BVH_NULL_NODE)
        return 0;

    uint32 leftHeight = CalculateHeight(mNodes[nodeID].child[0]);
    uint32 rightHeight = CalculateHeight(mNodes[nodeID].child[1]);
    return Max(leftHeight, rightHeight) + 1;
}

uint32 BVH::CalculateHeight() const
{
    return CalculateHeight(mRoot);
}

void BVH::GetNodeStats(uint32 nodeID, BVHStats* stats) const
{
    if (nodeID == NFE_BVH_NULL_NODE)
        return;

    stats->totalArea += (double)mNodes[nodeID].AABB.SurfaceArea();
    stats->totalVolume += (double)mNodes[nodeID].AABB.Volume();

    GetNodeStats(mNodes[nodeID].child[0], stats);
    GetNodeStats(mNodes[nodeID].child[1], stats);
}

void BVH::GetStats(BVHStats* stats) const
{
    stats->height = CalculateHeight();
    stats->nodesNum = mNodesNum;
    stats->leavesNum = mLeavesNum;

    stats->totalArea = 0.0;
    stats->totalVolume = 0.0;
    GetNodeStats(mRoot, stats);
}

uint32 BVH::GetRootId() const
{
    return mRoot;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Query functions ////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void BVH::QueryAll(uint32 nodeID, const QueryCallback& callback) const
{
    int stackDepth = 0;
    uint32 stack[NFE_BVH_STACK_SIZE];
    stack[stackDepth++] = nodeID;

    while (stackDepth > 0)
    {
        const BVHNode& node = mNodes[stack[--stackDepth]];

        if (node.IsLeaf())
        {
            callback(node.userData);
        }
        else if (stackDepth + 2 <= NFE_BVH_STACK_SIZE)
        {
            stack[stackDepth++] = node.child[0];
            stack[stackDepth++] = node.child[1];
        }
    }
}

template <>
NFCOMMON_API void BVH::Query(const Box& shape, const QueryCallback& callback) const
{
    if (mRoot == NFE_BVH_NULL_NODE)
        return;

    int stackDepth = 0;
    uint32 stack[NFE_BVH_STACK_SIZE];
    stack[stackDepth++] = mRoot;

    while (stackDepth > 0)
    {
        const BVHNode& node = mNodes[stack[--stackDepth]];

        if (Intersect(node.AABB, shape))
        {
            if (node.IsLeaf())
                callback(node.userData);
            else if (stackDepth + 2 <= NFE_BVH_STACK_SIZE)
            {
                stack[stackDepth++] = node.child[0];
                stack[stackDepth++] = node.child[1];
            }
        }
    }
}

template <>
NFCOMMON_API void BVH::Query(const Frustum& shape, const QueryCallback& callback) const
{
    if (mRoot == NFE_BVH_NULL_NODE)
        return;

    int stackDepth = 0;
    uint32 stack[NFE_BVH_STACK_SIZE];
    stack[stackDepth++] = mRoot;

    while (stackDepth > 0)
    {
        uint32 nodeID = stack[--stackDepth];
        const BVHNode& node = mNodes[nodeID];

        IntersectionResult result = IntersectEx(node.AABB, shape);
        if (result == IntersectionResult::Inside)
        {
            QueryAll(nodeID, callback);
            continue;
        }
        else if (result == IntersectionResult::Intersect)
        {
            if (node.IsLeaf())
            {
                callback(node.userData);
            }
            else if (stackDepth + 2 <= NFE_BVH_STACK_SIZE)
            {
                stack[stackDepth++] = node.child[0];
                stack[stackDepth++] = node.child[1];
            }
        }
    }
}

} // namespace Common
} // namespace NFE
