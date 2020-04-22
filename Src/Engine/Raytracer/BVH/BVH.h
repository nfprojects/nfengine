#pragma once

#include "../Raytracer.h"
#include "../Utils/Memory.h"
#include "../../Common/Math/Vec4f.hpp"
#include "../../Common/Math/Vec3f.hpp"
#include "../../Common/Math/Box.hpp"
#include "../../Common/Math/SimdBox.hpp"
#include "../../Common/Containers/DynArray.hpp"

namespace NFE {
namespace RT {

// binary Bounding Volume Hierarchy
class BVH
{
public:
    static constexpr uint32 MaxDepth = 128;

    struct NFE_ALIGN(32) Node
    {
        // TODO revisit this structure: keeping a pointer to child would be faster than index
        Math::Vec3f min;
        uint32 childIndex; // first child node / leaf index
        Math::Vec3f max;
        uint32 numLeaves : 30;
        uint32 splitAxis : 2;

        NFE_FORCE_INLINE const Math::Box GetBox() const
        {
            const Math::Vec4f mask = Math::Vec4f::MakeMask<1,1,1,0>();

            return { Math::Vec4f(&min.x) & mask, Math::Vec4f(&max.x) & mask };
        }

        template<typename VecType>
        NFE_FORCE_INLINE const Math::SimdBox<VecType> GetSimdBox() const
        {
            Math::SimdBox<VecType> ret;

            ret.min = VecType(min);
            ret.max = VecType(max);

            return ret;
        }

        NFE_FORCE_INLINE bool IsLeaf() const
        {
            return numLeaves != 0;
        }

        NFE_FORCE_INLINE uint32 GetSplitAxis() const
        {
            return splitAxis;
        }
    };

    struct Stats
    {
        uint32 maxDepth;    // max leaf depth
        double totalNodesArea;
        double totalNodesVolume;
        Common::DynArray<uint32> leavesCountHistogram;

        // TODO overlap factor, etc.

        Stats()
            : maxDepth(0)
            , totalNodesArea(0.0)
            , totalNodesVolume(0.0)
        { }
    };

    BVH();
    BVH(BVH&& rhs) = default;
    BVH& operator = (BVH&& rhs) = default;

    // calculate whole BVH stats
    void CalculateStats(Stats& outStats) const;

    bool SaveToFile(const std::string& filePath) const;
    bool LoadFromFile(const std::string& filePath);

    NFE_FORCE_INLINE const Node* GetNodes() const { return mNodes.Data(); }
    NFE_FORCE_INLINE uint32 GetNumNodes() const { return mNumNodes; }

private:
    void CalculateStatsForNode(uint32 node, Stats& outStats, uint32 depth) const;
    bool AllocateNodes(uint32 numNodes);

    Common::DynArray<Node> mNodes;
    uint32 mNumNodes;

    friend class BVHBuilder;
};


} // namespace RT
} // namespace NFE
