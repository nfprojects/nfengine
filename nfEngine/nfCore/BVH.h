/**
    NFEngine project

    \file   BVH.h
    \brief  Bounding Volume Hierarchy definitions.
*/

#pragma once

#include "Core.h"

namespace NFE {
namespace Util {

NFE_ALIGN(16)
struct BVHNode
{
    Math::Box AABB;

    union
    {
        uint32 parent;

        // next free node in free list
        uint32 next;
    };

    uint32 child[2];
    void* userData;

    void* operator new(size_t size);
    void operator delete(void* ptr);
    BVHNode();
    bool IsLeaf() const;
};

struct BVHStats
{
    uint32 leavesNum;
    uint32 nodesNum;
    uint32 height;

    double totalArea;
    double totatlVolume;
};

typedef void (*BVHQueryCallback)(void*, void*); //void *pLeafUserData, void *pCallbackUserData


/*
    Dynamic bounding volume herarchy
    Implementation based on Box2d b2DynamicTree
*/
class CORE_API BVH
{
    // free list
    BVHNode* mNodes;
    uint32 mNodesNum;
    uint32 mNodesCapacity;
    uint32 mFreeNode;

    uint32 mRoot;
    uint32 mLeavesNum;

    uint32 mRebalancePath;


    void InsertLeaf(uint32 leaf);
    void RemoveLeaf(uint32 leaf);

    uint32 AllocNode();
    void FreeNode(uint32 nodeID);

    void QueryAll(uint32 node, BVHQueryCallback pCallback, void* pUserData) const;

public:
    BVH();
    ~BVH();

    // remove all nodes
    void Clear();

    // get number of inserted leaves
    uint32 GetSize() const;

    // Insert a new leaf to the tree. About O(log n) complexity
    uint32 Insert(const Math::Box& aabb, void* pUserData);

    // Remove an leaf from the tree
    bool Remove(uint32 nodeID);

    // Change leaf's AABB
    bool Move(uint32 nodeID, const Math::Box& aabb);

    void Rebalance(uint32 iterations);

    // calculate height starting from nodeID
    uint32 CalculateHeight(uint32 nodeID) const;

    // calcualte height of whole tree
    uint32 CalculateHeight() const;

    void GetNodeStats(uint32 nodeID, BVHStats* pStats) const;
    void GetStats(BVHStats* pStats) const;

    // Perform query: each leaf colliding with 'shape' will be passed as a parameter in the callback.
    // New query shapes can be easly added by specialization of this template.
    template <typename ShapeType>
    void Query(const ShapeType& shape, BVHQueryCallback pCallback, void* pUserData) const;

    // get root node ID
    uint32 GetRootId() const;

    // get node object
    BVHNode* GetNodeById(uint32 id) const;
};

} // namespace Util
} // namespace NFE
