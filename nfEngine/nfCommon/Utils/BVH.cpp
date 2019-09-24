/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Bounding Volume Hierarchy declaration.
 */

#include "PCH.hpp"
#include "BVH.hpp"

namespace NFE {
namespace Common {

using namespace Math;

#define NFE_BVH_INITIAL_CAPACITY 128

BVHNode::BVHNode()
{
    parent = NFE_BVH_NULL_NODE;
    userData = nullptr;
    child0 = child1 = NFE_BVH_NULL_NODE;
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
    mNodes.Resize(mNodesCapacity);

    // init free list
    for (uint32 i = 0; i < mNodesCapacity - 1; i++)
    {
        mNodes[i].height = NFE_BVH_NULL_NODE;
        mNodes[i].next = i + 1;
    }
    mNodes[mNodesCapacity - 1].next = NFE_BVH_NULL_NODE;
    mNodes[mNodesCapacity - 1].height = NFE_BVH_NULL_NODE;
    mFreeNode = 0;
}

// allocate space for a new node from the free list
uint32 BVH::AllocNode()
{
    // expand free list
    if (mFreeNode == NFE_BVH_NULL_NODE)
    {
        mNodesCapacity *= 2;
        mNodes.Resize(mNodesCapacity);

        for (uint32 i = mNodesNum; i < mNodesCapacity - 1; i++)
        {
            mNodes[i].next = i + 1;
            mNodes[i].height = NFE_BVH_NULL_NODE;
        }
        mNodes[mNodesCapacity - 1].next = NFE_BVH_NULL_NODE;
        mFreeNode = mNodesNum;
    }

    int id = mFreeNode;
    mFreeNode = mNodes[mFreeNode].next;
    mNodes[id].parent = NFE_BVH_NULL_NODE;
    mNodes[id].child0 = NFE_BVH_NULL_NODE;
    mNodes[id].child1 = NFE_BVH_NULL_NODE;
    mNodes[id].height = 0;

    mNodesNum++;
    return id;
}

// mark node as free
void BVH::FreeNode(uint32 nodeID)
{
    mNodes[nodeID].next = mFreeNode;
    mNodes[nodeID].height = NFE_BVH_NULL_NODE;
    mFreeNode = nodeID;
    mNodesNum--;
}

uint32 BVH::Insert(const Box& aabb, void* userData)
{
    uint32 leaf = AllocNode();
    mNodes[leaf].SetBox(aabb);
    mNodes[leaf].userData = userData;
    mNodes[leaf].child0 = NFE_BVH_NULL_NODE;
    mNodes[leaf].child1 = NFE_BVH_NULL_NODE;
    InsertLeaf(leaf);

    mLeavesNum++;
    return leaf;
}

bool BVH::Remove(uint32 nodeID)
{
    if (nodeID >= mNodesCapacity || !mNodes[nodeID].IsValid() || !mNodes[nodeID].IsLeaf())
        return false;

    RemoveLeaf(nodeID);
    FreeNode(nodeID);
    mLeavesNum--;
    return true;
}

bool BVH::Move(uint32 nodeID, const Box& aabb)
{
    if (nodeID >= mNodesCapacity || !mNodes[nodeID].IsValid() || !mNodes[nodeID].IsLeaf())
        return false;

    RemoveLeaf(nodeID);
    mNodes[nodeID].SetBox(aabb);
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

    const Box leafBox = mNodes[leaf].GetBox();
    uint32 index = mRoot;
    while (!mNodes[index].IsLeaf())
    {
        uint32 child0 = mNodes[index].child0;
        uint32 child1 = mNodes[index].child1;

        const Box currentBox = mNodes[index].GetBox();
        const Box childBox0 = mNodes[child0].GetBox();
        const Box childBox1 = mNodes[child1].GetBox();

        float cost = 2.0f * currentBox.Volume();
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

        if (cost < cost0 && cost < cost1)
            break;

        index = cost0 < cost1 ? child0 : child1;
    }

    // create internal parent node
    uint32 newParent = AllocNode();
    uint32 oldParent = mNodes[index].parent;
    mNodes[newParent].parent = oldParent;
    mNodes[newParent].SetBox(Box(mNodes[leaf].GetBox(), mNodes[index].GetBox()));
    mNodes[newParent].height = 1 + mNodes[index].height;

    if (oldParent != NFE_BVH_NULL_NODE)
    {
        if (mNodes[mNodes[index].parent].child0 == index)
            mNodes[oldParent].child0 = newParent;
        else
            mNodes[oldParent].child1 = newParent;
    }
    else
        mRoot = newParent;

    mNodes[newParent].child0 = index;
    mNodes[newParent].child1 = leaf;
    mNodes[index].parent = newParent;
    mNodes[leaf].parent = newParent;

    // correct boxes and heights up to the root
    index = mNodes[leaf].parent;
    while (index != NFE_BVH_NULL_NODE)
    {
        index = Rebalance(index);

        uint32 child0 = mNodes[index].child0;
        uint32 child1 = mNodes[index].child1;

        mNodes[index].height = 1 + Max(mNodes[child0].height, mNodes[child1].height);
        mNodes[index].SetBox(Box(mNodes[child0].GetBox(), mNodes[child1].GetBox()));
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
    if (mNodes[parent].child0 == leaf)
        sibling = mNodes[parent].child1;
    else
        sibling = mNodes[parent].child0;

    uint32 grandParent = mNodes[parent].parent;
    if (grandParent != NFE_BVH_NULL_NODE)
    {
        // destroy parent and connect sibling to grandparent
        if (mNodes[grandParent].child0 == parent)
            mNodes[grandParent].child0 = sibling;
        else
            mNodes[grandParent].child1 = sibling;

        mNodes[sibling].parent = grandParent;
        FreeNode(parent);

        // correct boxes and heights up to the root
        uint32 index = grandParent;
        while (index != NFE_BVH_NULL_NODE)
        {
            index = Rebalance(index);

            uint32 child0 = mNodes[index].child0;
            uint32 child1 = mNodes[index].child1;

            mNodes[index].height = 1 + Max(mNodes[child0].height, mNodes[child1].height);
            mNodes[index].SetBox(Box(mNodes[child0].GetBox(), mNodes[child1].GetBox()));
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

int32 BVH::Rebalance(int32 node)
{
    uint32 iA = node;

    BVHNode& A = mNodes[iA];
    if (A.IsLeaf() || A.height < 2)
        return iA;

    const int maxImbalance = 1 + static_cast<int>(A.height) / 2;

    int32 iB = A.child0;
    int32 iC = A.child1;

    BVHNode& B = mNodes[iB];
    BVHNode& C = mNodes[iC];

    int balance = static_cast<int>(C.height) - static_cast<int>(B.height);
    if (balance > maxImbalance) // Rotate C up
    {
        int32 iF = C.child0;
        int32 iG = C.child1;
        BVHNode& F = mNodes[iF];
        BVHNode& G = mNodes[iG];

        // Swap A and C
        C.child0 = iA;
        C.parent = A.parent;
        A.parent = iC;

        // A's old parent should point to C
        if (C.parent != NFE_BVH_NULL_NODE)
        {
            if (mNodes[C.parent].child0 == iA)
                mNodes[C.parent].child0 = iC;
            else
                mNodes[C.parent].child1 = iC;
        }
        else
            mRoot = iC;

        // Rotate
        if (F.height > G.height)
        {
            C.child1 = iF;
            A.child1 = iG;
            G.parent = iA;
            A.SetBox(Box(B.GetBox(), G.GetBox()));
            C.SetBox(Box(A.GetBox(), F.GetBox()));

            A.height = 1 + Max(B.height, G.height);
            C.height = 1 + Max(A.height, F.height);
        }
        else
        {
            C.child1 = iG;
            A.child1 = iF;
            F.parent = iA;
            A.SetBox(Box(B.GetBox(), F.GetBox()));
            C.SetBox(Box(A.GetBox(), G.GetBox()));

            A.height = 1 + Max(B.height, F.height);
            C.height = 1 + Max(A.height, G.height);
        }

        return iC;
    }
    else if (balance < -maxImbalance) // Rotate B up
    {
        int32 iD = B.child0;
        int32 iE = B.child1;
        BVHNode& D = mNodes[iD];
        BVHNode& E = mNodes[iE];

        // Swap A and B
        B.child0 = iA;
        B.parent = A.parent;
        A.parent = iB;

        // A's old parent should point to B
        if (B.parent != NFE_BVH_NULL_NODE)
        {
            if (mNodes[B.parent].child0 == iA)
                mNodes[B.parent].child0 = iB;
            else
                mNodes[B.parent].child1 = iB;
        }
        else
            mRoot = iB;

        // Rotate
        if (D.height > E.height)
        {
            B.child1 = iD;
            A.child0 = iE;
            E.parent = iA;
            A.SetBox(Box(C.GetBox(), E.GetBox()));
            B.SetBox(Box(A.GetBox(), D.GetBox()));

            A.height = 1 + Max(C.height, E.height);
            B.height = 1 + Max(A.height, D.height);
        }
        else
        {
            B.child1 = iE;
            A.child0 = iD;
            D.parent = iA;
            A.SetBox(Box(C.GetBox(), D.GetBox()));
            B.SetBox(Box(A.GetBox(), E.GetBox()));

            A.height = 1 + Max(C.height, D.height);
            B.height = 1 + Max(A.height, E.height);
        }

        return iB;
    }

    return iA;
}

void BVH::GetNodeStats(uint32 nodeID, BVHStats& stats) const
{
    if (nodeID == NFE_BVH_NULL_NODE)
        return;

    // Internal and leaves' boxes can have very different sizes, so caclulate
    // the sums in double precission to avoid numeric precission problems.
    stats.totalArea += static_cast<double>(mNodes[nodeID].GetBox().SurfaceArea());
    stats.totalVolume += static_cast<double>(mNodes[nodeID].GetBox().Volume());

    GetNodeStats(mNodes[nodeID].child0, stats);
    GetNodeStats(mNodes[nodeID].child1, stats);
}

void BVH::GetStats(BVHStats& stats) const
{
    if (mRoot != NFE_BVH_NULL_NODE)
        stats.height = mNodes[mRoot].height;
    else
        stats.height = 0;

    stats.nodesNum = mNodesNum;
    stats.leavesNum = mLeavesNum;

    stats.totalArea = 0.0;
    stats.totalVolume = 0.0;
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
            callback(node.userData);
        else if (stackDepth + 2 <= NFE_BVH_STACK_SIZE)
        {
            stack[stackDepth++] = node.child0;
            stack[stackDepth++] = node.child1;
        }
    }
}

} // namespace Common
} // namespace NFE
