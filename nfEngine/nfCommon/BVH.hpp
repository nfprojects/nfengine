/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Bounding Volume Hierarchy definitions.
 */

#pragma once

#include "nfCommon.hpp"
#include "Math/Box.hpp"
#include "Math/Geometry.hpp"
#include "Aligned.hpp"

#define NFE_BVH_STACK_SIZE 128
#define NFE_BVH_NULL_NODE 0xFFFFFFFF


// TODO temporary workaround
#pragma warning(disable : 4201)

namespace NFE {
namespace Common {

struct NFE_ALIGN16 BVHNode
{
    union
    {
        Math::Box box;

        struct
        {
            float boxMin[3]; // not used directly
            uint32 child0;   // set to NFE_BVH_NULL_NODE when the node is leaf node
            float boxMax[3]; // not used directly
            uint32 child1;
        };
    };

    // TODO: move to separate arrays
    void* userData;
    uint32 height; // node height (counting from leaves)

    union
    {
        uint32 parent; // parent node ID
        uint32 next;   // next free node in free list
    };

    BVHNode();

    NFE_INLINE void SetBox(const Math::Box& newBox)
    {
        uint32 tmp0 = child0;
        uint32 tmp1 = child1;
        box = newBox;
        child0 = tmp0;
        child1 = tmp1;
    }

    NFE_INLINE bool IsValid() const
    {
        return height != NFE_BVH_NULL_NODE;
    }

    NFE_INLINE bool IsLeaf() const
    {
        return child0 == NFE_BVH_NULL_NODE;
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
    std::vector<BVHNode, AlignedAllocator<BVHNode, 32>> mNodes;
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

    /**
     * Perform tree rotation if @p node is imbalanced.
     * @return New node index
     */
    int32 Rebalance(int32 node);

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


template <typename ShapeType>
__declspec(noinline)
void BVH::Query(const ShapeType& shape, const QueryCallback& callback) const
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

        IntersectionResult result = IntersectEx(node.box, shape);
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
                stack[stackDepth++] = node.child0;
                stack[stackDepth++] = node.child1;
            }
        }
    }
}

} // namespace Common
} // namespace NFE
