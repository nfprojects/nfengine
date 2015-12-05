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

namespace NFE {
namespace Common {

struct NFE_ALIGN16 BVHNode : public Aligned<16>
{
    Math::Box box;
    void* userData;

    uint32 child[2]; // child nodes or NFE_BVH_NULL_NODE when the node is leaf node
    uint32 height; // node height (counting from leaves)

    union
    {
        uint32 parent; // parent node ID
        uint32 next;   // next free node in free list
    };

    BVHNode();

    NFE_INLINE bool IsValid() const
    {
        return height != NFE_BVH_NULL_NODE;
    }

    NFE_INLINE bool IsLeaf() const
    {
        return child[0] == NFE_BVH_NULL_NODE;
    }
};

struct BVHStats
{
    double totalArea;
    double totalVolume;

    uint32 leavesNum;
    uint32 nodesNum;
    uint32 height;
};

/**
 * Dynamic bounding volume herarchy.
 * Implementation based on Box2d b2DynamicTree.
 */
class NFCOMMON_API BVH
{
    // free list
    std::vector<BVHNode, AlignedAllocator<BVHNode, 64>> mNodes;
    uint32 mNodesNum;
    uint32 mNodesCapacity;
    uint32 mFreeNode;

    uint32 mRoot;
    uint32 mLeavesNum;

    // make an allocated node a leaf
    void InsertLeaf(uint32 leaf);
    void RemoveLeaf(uint32 leaf);

    uint32 AllocNode();
    void FreeNode(uint32 nodeID);

    /**
     * Get root node ID.
     */
    NFE_INLINE uint32 GetRootId() const
    {
        return mRoot;
    }

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
     * Get statistics of a given node.
     * @param nodeID     Node ID for which the stats is calculated.
     * @param[out] stats Result.
     */
    void GetNodeStats(uint32 nodeID, BVHStats& stats) const;

    /**
     * Get statistics of the whole tree.
     * @param nodeID     Node ID for which the stats is calculated.
     * @param[out] stats Result.
     */
    void GetStats(BVHStats& stats) const;

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
};

} // namespace Common
} // namespace NFE
