#pragma once

#include "../Raytracer.h"
#include "../Utils/Memory.h"
#include "../../nfCommon/Math/Vector4.hpp"
#include "../../nfCommon/Math/Float3.hpp"
#include "../../nfCommon/Math/Box.hpp"
#include "../../nfCommon/Math/Simd8Box.hpp"
#include "../../nfCommon/Containers/DynArray.hpp"

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
        Math::Float3 min;
        uint32 childIndex; // first child node / leaf index
        Math::Float3 max;
        uint32 numLeaves : 30;
        uint32 splitAxis : 2;

        NFE_FORCE_INLINE const Math::Box GetBox() const
        {
            const Math::Vector4 mask = Math::Vector4::MakeMask<1,1,1,0>();

            return { Math::Vector4(&min.x) & mask, Math::Vector4(&max.x) & mask };
        }

        NFE_FORCE_INLINE Math::Box_Simd8 GetBox_Simd8() const
        {
            Math::Box_Simd8 ret;

            ret.min.x = Math::Vector8(min.x);
            ret.min.y = Math::Vector8(min.y);
            ret.min.z = Math::Vector8(min.z);

            ret.max.x = Math::Vector8(max.x);
            ret.max.y = Math::Vector8(max.y);
            ret.max.z = Math::Vector8(max.z);

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
