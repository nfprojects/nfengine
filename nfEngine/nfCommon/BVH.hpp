/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Bounding Volume Hierarchy definitions.
 */

#pragma once

#include "nfCommon.hpp"
#include "Math/Box.hpp"

namespace NFE {
namespace Common {

NFE_ALIGN16
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
    double totalVolume;
};

typedef void (*BVHQueryCallback)(void*, void*); //void *leafUserData, void *callbackUserData


/*
    Dynamic bounding volume herarchy
    Implementation based on Box2d b2DynamicTree
*/
class NFCOMMON_API BVH
{
    // free list
    BVHNode* mNodes;
    uint32 mNodesNum;
    uint32 mNodesCapacity;
    uint32 mFreeNode;

    uint32 mRoot;
    uint32 mLeavesNum;

    uint32 mRebalancePath;

    BVHNode* GetNodeById(uint32 id) const;
    void InsertLeaf(uint32 leaf);
    void RemoveLeaf(uint32 leaf);

    uint32 AllocNode();
    void FreeNode(uint32 nodeID);

    void QueryAll(uint32 nodeID, BVHQueryCallback callback, void* userData) const;

public:
    BVH();
    ~BVH();

    /**
     * Remove all tree nodes.
     */
    void Clear();

    /**
     * Get number of inserted leaves.
     */
    uint32 GetSize() const;

    /**
     * Insert a new leaf to the tree.
     * @note O(log N) average complexity.
     *
     * @param aabb     Leaf axis-aligned bounding box.
     * @param userData Optional user data bounded to the leaf.
     * @return         Leaf node ID.
     */
    uint32 Insert(const Math::Box& aabb, void* userData);

    /**
     * Remove a leaf from the tree.
     */
    bool Remove(uint32 nodeID);

    /**
     * Change leaf's AABB.
     * @param nodeID   Leaf node ID.
     * @param aabb     New leaf axis-aligned bounding box.
     * @raturn True on success.
     */
    bool Move(uint32 nodeID, const Math::Box& aabb);

    /**
     * Rebalance the tree to improve query performance.
     */
    void Rebalance(uint32 iterations);

    /**
     * Calculate tree height starting from a given node.
     * @param nodeID Node ID for which the height is calculated.
     */
    uint32 CalculateHeight(uint32 nodeID) const;

    /**
     * Calculate whole tree height.
     */
    uint32 CalculateHeight() const;

    /**
     * Get statistics of a given node.
     * @param nodeID     Node ID for which the stats is calculated.
     * @param[out] stats Result.
     */
    void GetNodeStats(uint32 nodeID, BVHStats* stats) const;

    /**
     * Get statistics of the whole tree.
     * @param nodeID     Node ID for which the stats is calculated.
     * @param[out] stats Result.
     */
    void GetStats(BVHStats* stats) const;

    /**
     * Perform query - each leaf colliding with 'shape' will be passed as a parameter
     * in the provided callback.
     * @note New query shapes can be easly added by specialization of this template.
     *
     * @param shape    Query area.
     * @param callback Query callback.
     * @param userData User data passed to query callback.
     */
    template <typename ShapeType>
    void Query(const ShapeType& shape, BVHQueryCallback callback, void* userData) const;

    /**
     * Get root node ID.
     */
    uint32 GetRootId() const;
};

} // namespace Common
} // namespace NFE
