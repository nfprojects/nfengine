#pragma once

#include "Raytracer.h"
#include "BVH.h"

namespace NFE {
namespace RT {

struct BvhBuildingParams
{
    enum class Heuristics
    {
        SurfaceArea,
        Volume
    };

    uint32 maxLeafNodeSize = 2; // max number of objects in leaf nodes
    Heuristics heuristics = Heuristics::SurfaceArea;
};

// helper class for constructing BVH using SAH algorithm
class BVHBuilder
{
public:

    using Indices = Common::DynArray<uint32>;

    BVHBuilder(BVH& targetBVH);
    ~BVHBuilder();

    void SetLeafData();

    // construct the BVH and return new leaves order
    bool Build(const Math::Box* data, const uint32 numLeaves, const BvhBuildingParams& params, Indices& outLeavesOrder);

private:

    constexpr static uint32 NumAxes = 3;

    struct Context
    {
        Common::DynArray<Math::Box> mLeftBoxesCache;
        Common::DynArray<Math::Box> mRightBoxesCache;
        Indices mSortedLeavesIndicesCache[3];

        Context(uint32 numLeaves);
    };

    struct NFE_ALIGN(16) WorkSet
    {
        Math::Box box;
        Indices leafIndices;
        uint32 numLeaves;
        uint32 sortedBy;
        uint32 depth;

        WorkSet()
            : numLeaves(0)
            , sortedBy(std::numeric_limits<uint32>::max())
            , depth(0)
        { }
    };

    // sort leaf indices in each axis
    void SortLeaves(const WorkSet& workSet, Context& context) const;
    void BuildNode(const WorkSet& workSet, Context& context, BVH::Node& targetNode);
    void GenerateLeaf(const WorkSet& workSet, BVH::Node& targetNode);

    // input data
    BvhBuildingParams mParams;
    const Math::Box* mLeafBoxes;
    uint32 mNumLeaves;

    uint32 mNumGeneratedNodes;
    uint32 mNumGeneratedLeaves;
    Indices mLeavesOrder;

    // target BVH
    BVH& mTarget;
};


} // namespace RT
} // namespace NFE
