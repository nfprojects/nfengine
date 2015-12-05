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
    parent = NFE_BVH_NULL_NODE;
    userData = nullptr;
    child[0] = child[1] = NFE_BVH_NULL_NODE;
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


void BVH::InsertLeaf(uint32 leaf)
{
    // empty tree => make node as root
    if (mRoot == NFE_BVH_NULL_NODE)
    {
        mRoot = leaf;
        mNodes[mRoot].parent = NFE_BVH_NULL_NODE;
        return;
    }

    const Box& leafBox = mNodes[leaf].AABB;
    uint32 index = mRoot;
    while (!mNodes[index].IsLeaf())
    {
        uint32 child0 = mNodes[index].child[0];
        uint32 child1 = mNodes[index].child[1];

        const Box& childBox0 = mNodes[child0].AABB;
        const Box& childBox1 = mNodes[child1].AABB;

        float cost0, cost1;

        // cost of descending into child0
        if (mNodes[child0].IsLeaf())
            cost0 = childBox0.Volume();
        else
        {
            Box newBox = Box(leafBox, childBox0);
            cost0 = newBox.Volume() - childBox0.Volume();
        }

        // cost of descending into child1
        if (mNodes[child1].IsLeaf())
            cost1 = childBox1.Volume();
        else
        {
            Box newBox = Box(leafBox, childBox1);
            cost1 = newBox.Volume() - childBox1.Volume();
        }

        index = cost0 < cost1 ? child0 : child1;
    }

    // create internal parent node
    uint32 newParent = AllocNode();
    uint32 oldParent = mNodes[index].parent;
    mNodes[newParent].parent = oldParent;
    mNodes[newParent].AABB = Box(mNodes[leaf].AABB, mNodes[index].AABB);

    if (oldParent != NFE_BVH_NULL_NODE)
    {
        if (mNodes[mNodes[index].parent].child[0] == index)
            mNodes[oldParent].child[0] = newParent;
        else
            mNodes[oldParent].child[1] = newParent;
    }
    else
        mRoot = newParent;

    mNodes[newParent].child[0] = index;
    mNodes[newParent].child[1] = leaf;
    mNodes[index].parent = newParent;
    mNodes[leaf].parent = newParent;

    // correct AABB up to the root
    index = mNodes[leaf].parent;
    while (index != NFE_BVH_NULL_NODE)
    {
        // TODO: tree rebalancing

        uint32 child0 = mNodes[index].child[0];
        uint32 child1 = mNodes[index].child[1];

        mNodes[index].AABB = Box(mNodes[child0].AABB, mNodes[child1].AABB);
        index = mNodes[index].parent;
    }
}

void BVH::RemoveLeaf(uint32 leaf)
{
    if (mRoot == leaf)
    {
        mRoot = NFE_BVH_NULL_NODE;
        return;
    }

    uint32 parent = mNodes[leaf].parent;
    uint32 sibling;
    if (mNodes[parent].child[0] == leaf)
        sibling = mNodes[parent].child[1];
    else
        sibling = mNodes[parent].child[0];

    uint32 grandParent = mNodes[parent].parent;
    if (grandParent != NFE_BVH_NULL_NODE)
    {
        // destroy parent and connect sibling to grandparent
        if (mNodes[grandParent].child[0] == parent)
            mNodes[grandParent].child[0] = sibling;
        else
            mNodes[grandParent].child[1] = sibling;

        mNodes[sibling].parent = grandParent;
        FreeNode(parent);

        // correct AABB up to the root
        uint32 index = grandParent;
        while (index != NFE_BVH_NULL_NODE)
        {
            // TODO: tree rebalancing

            uint32 child0 = mNodes[index].child[0];
            uint32 child1 = mNodes[index].child[1];

            mNodes[index].AABB = Box(mNodes[child0].AABB, mNodes[child1].AABB);
            index = mNodes[index].parent;
        }
    }
    else
    {
        mRoot = sibling;
        mNodes[sibling].parent = NFE_BVH_NULL_NODE;
        FreeNode(parent);
    }
}

uint32 BVH::CalculateHeight(uint32 nodeID) const
{
    if (nodeID == NFE_BVH_NULL_NODE || nodeID == NFE_BVH_LEAF_NODE)
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
    if (nodeID == NFE_BVH_NULL_NODE || nodeID == NFE_BVH_LEAF_NODE)
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
