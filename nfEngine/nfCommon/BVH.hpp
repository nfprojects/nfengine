/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Bounding Volume Hierarchy definitions.
 */

#pragma once

#include "nfCommon.hpp"
#include "Math/Box.hpp"
#include "Aligned.hpp"


#define NFE_BVH_NULL_NODE 0xFFFFFFFF
#define NFE_BVH_LEAF_NODE 0xFFFFFFFE

namespace NFE {
namespace Common {

struct NFE_ALIGN16 BVHNode : public Aligned<16>
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

    BVHNode();

    NFE_INLINE bool IsLeaf() const
    {
        return child[0] == NFE_BVH_LEAF_NODE;
    }
};

struct BVHStats
{
    uint32 leavesNum;
    uint32 nodesNum;
    uint32 height;

    double totalArea;
    double totalVolume;
};

/**
 * Dynamic bounding volume herarchy.
 * Implementation based on Box2d b2DynamicTree.
 */
class NFCOMMON_API BVH
{
    // free list
    std::vector<BVHNode, AlignedAllocator<BVHNode, 16>> mNodes;
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

public:
    typedef std::function<void(void *leafUserData)> QueryCallback;

    BVH();

    /**
     * Remove all tree nodes.
     */
    void Clear();

    /**
     * Get number of inserted leaves.
     */
    NFE_INLINE uint32 GetSize() const
    {
        return mLeavesNum;
    }

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
    void Query(const ShapeType& shape, const QueryCallback& callback) const;

    /**
     * Query all leaves within a node.
     */
    void QueryAll(uint32 nodeID, const QueryCallback& callback) const;

    /**
     * Get root node ID.
     */
    uint32 GetRootId() const;
};

} // namespace Common
} // namespace NFE
