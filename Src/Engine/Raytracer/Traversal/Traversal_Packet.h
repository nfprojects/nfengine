#pragma once

#include "RayPacket.h"
#include "HitPoint.h"
#include "TraversalContext.h"
#include "BVH/BVH.h"
#include "Utils/iacaMarks.h"
#include "Rendering/Counters.h"
#include "Rendering/RenderingContext.h"
#include "../../Common/Math/Ray.hpp"
#include "../../Common/Math/Geometry.hpp"
#include "../../Common/Math/SimdGeometry.hpp"

// #define NFE_NO_RAY_REORDERING

namespace NFE {
namespace RT {

// remove groups where all rays missed a bounding box
NFE_FORCE_NOINLINE uint32 RemoveMissedGroups(RenderingContext& context, uint32 numGroups);

// reorder rays to restore coherency
NFE_FORCE_NOINLINE void ReorderRays(RenderingContext& context, uint32 numRays, uint32 traversalDepth);

// test all alive groups in a packet agains a BVH node
NFE_FORCE_NOINLINE uint32 TestRayPacket(RayPacket& packet, uint32 numGroups, const BVH::Node& node, RenderingContext& context, uint32 traversalDepth);

template <typename ObjectType, uint32 traversalDepth>
NFE_FORCE_NOINLINE void GenericTraverse(const PacketTraversalContext& context, const uint32 objectID, const ObjectType* object, uint32 numActiveGroups)
{
    // all nodes
    const BVH::Node* __restrict nodes = object->GetBVH().GetNodes();

    struct StackFrame
    {
        const BVH::Node* node;
        uint32 numActiveGroups;
        uint32 numActiveRays;
    };

    StackFrame stack[BVH::MaxDepth];

    // push root
    uint32 stackSize = 1;
    stack[0].node = nodes;
    stack[0].numActiveGroups = numActiveGroups;
    stack[0].numActiveRays = context.ray.numRays; // all rays are active at the beginning

    // TODO packets should be octant-sorted
    uint32 rayOctant = 0;
    rayOctant = context.ray.groups[0].rays[traversalDepth].dir.x[0] < 0.0f ? 1 : 0;
    rayOctant |= context.ray.groups[0].rays[traversalDepth].dir.y[0] < 0.0f ? 2 : 0;
    rayOctant |= context.ray.groups[0].rays[traversalDepth].dir.z[0] < 0.0f ? 4 : 0;

    // BVH traversal
    while (stackSize > 0)
    {
        // pop element from stack
        const StackFrame& frame = stack[--stackSize];

        uint32 numGroups = frame.numActiveGroups;
        uint32 raysHit = TestRayPacket(context.ray, numGroups, *frame.node, context.context, traversalDepth);

#ifdef NFE_ENABLE_INTERSECTION_COUNTERS
        context.context.localCounters.numRayBoxTests += RayPacketTypes::GroupSize * numGroups;
        context.context.localCounters.numPassedRayBoxTests += raysHit;
#endif // NFE_ENABLE_INTERSECTION_COUNTERS

        if (raysHit == 0)
        {
            // all rays missed the node - skip it
            continue;
        }

        // remove missed groups from the list
        if (raysHit < frame.numActiveRays)
        {
            numGroups = RemoveMissedGroups(context.context, numGroups);

#ifndef NFE_NO_RAY_REORDERING
            // reorder rays to restore coherency
            if ((numGroups > 1) && ((RayPacketTypes::GroupSize / 4u * numGroups) >= raysHit)) // 25% utilization
            {
                ReorderRays(context.context, numGroups, traversalDepth);
                numGroups = (raysHit + RayPacketTypes::GroupSize - 1u) / RayPacketTypes::GroupSize;
            }
#endif // NFE_NO_RAY_REORDERING
        }

        // TODO switching to Simd traversal if only one group left

        if (frame.node->IsLeaf())
        {
            object->Traverse_Leaf(context, objectID, *frame.node, numGroups);
        }
        else
        {
            const BVH::Node* __restrict children = nodes + frame.node->childIndex;
            NFE_PREFETCH_L1(children);

            // stored split axis trick: pust stack elements based on current node's split axis
            const uint32 firstIndex = (rayOctant >> frame.node->GetSplitAxis()) & 1u;
            const uint32 secondIndex = firstIndex ^ 1u;

            stack[stackSize].node = children + secondIndex;
            stack[stackSize].numActiveGroups = numGroups;
            stack[stackSize].numActiveRays = raysHit;
            stackSize++;

            stack[stackSize].node = children + firstIndex;
            stack[stackSize].numActiveGroups = numGroups;
            stack[stackSize].numActiveRays = raysHit;
            stackSize++;
        }
    }
}

} // namespace RT
} // namespace NFE
