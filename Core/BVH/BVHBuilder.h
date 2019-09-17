#pragma once

#include "RayLib.h"
#include "BVH.h"

namespace rt {

struct BvhBuildingParams
{
    enum class Heuristics
    {
        SurfaceArea,
        Volume
    };

    Uint32 maxLeafNodeSize = 2; // max number of objects in leaf nodes
    Heuristics heuristics = Heuristics::SurfaceArea;
};

// helper class for constructing BVH using SAH algorithm
class BVHBuilder
{
public:

    using Indices = DynArray<Uint32>;

    BVHBuilder(BVH& targetBVH);
    ~BVHBuilder();

    void SetLeafData();

    // construct the BVH and return new leaves order
    bool Build(const math::Box* data, const Uint32 numLeaves, const BvhBuildingParams& params, Indices& outLeavesOrder);

private:

    constexpr static Uint32 NumAxes = 3;

    struct Context
    {
        DynArray<math::Box> mLeftBoxesCache;
        DynArray<math::Box> mRightBoxesCache;
        Indices mSortedLeavesIndicesCache[3];

        Context(Uint32 numLeaves);
    };

    struct RT_ALIGN(16) WorkSet
    {
        math::Box box;
        Indices leafIndices;
        Uint32 numLeaves;
        Uint32 sortedBy;
        Uint32 depth;

        WorkSet()
            : numLeaves(0)
            , sortedBy(std::numeric_limits<Uint32>::max())
            , depth(0)
        { }
    };

    // sort leaf indices in each axis
    void SortLeaves(const WorkSet& workSet, Context& context) const;
    void BuildNode(const WorkSet& workSet, Context& context, BVH::Node& targetNode);
    void GenerateLeaf(const WorkSet& workSet, BVH::Node& targetNode);

    // input data
    BvhBuildingParams mParams;
    const math::Box* mLeafBoxes;
    Uint32 mNumLeaves;

    Uint32 mNumGeneratedNodes;
    Uint32 mNumGeneratedLeaves;
    Indices mLeavesOrder;

    // target BVH
    BVH& mTarget;
};


} // namespace rt
