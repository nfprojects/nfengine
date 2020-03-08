#pragma once

#include "Raytracer.h"
#include "BVH.h"
#include "../../Common/Containers/SharedPtr.hpp"

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

    struct NFE_ALIGN(64) ThreadData
    {
        Common::DynArray<Math::Box> mLeftBoxesCache;
        Common::DynArray<Math::Box> mRightBoxesCache;

        void Init(uint32 numLeaves);
    };

    struct NFE_ALIGN(16) WorkSet
    {
        Math::Box box;
        Indices leafIndices;
        Indices sortedLeavesIndicesCache[NumAxes];
        uint32 numLeaves;
        uint32 sortedBy;
        uint32 depth;

        WorkSet()
            : numLeaves(0)
            , sortedBy(std::numeric_limits<uint32>::max())
            , depth(0)
        { }
    };

    using WorkSetPtr = Common::SharedPtr<WorkSet>;

    void SubdivideNode(ThreadData& threadData, const WorkSet& workSet,
        uint32& outAxis, uint32& outSplitPos, Math::Box& outLeftBox, Math::Box& outRightBox) const;

    // sort leaf indices in each axis
    void SortLeavesInAxis(Indices& indicesToSort, uint32 axis) const;

    // sort leaf indices in each axis
    void SortLeaves(WorkSet& workSet) const;
    void SortLeaves_Threaded(const WorkSetPtr& workSet, Common::TaskBuilder& taskBuilder) const;

    void BuildNode(ThreadData& threadData, WorkSet& workSet, BVH::Node& targetNode);
    void BuildNode_Threaded(const WorkSetPtr& workSet, BVH::Node& targetNode, const Common::TaskContext& taskContext, Common::TaskBuilder& taskBuilder);

    void GenerateLeaf(const WorkSet& workSet, BVH::Node& targetNode);

    // target BVH
    BVH& mTarget;

    // input data
    BvhBuildingParams mParams;
    const Math::Box* mLeafBoxes;
    uint32 mNumLeaves;

    Common::DynArray<ThreadData> mThreadData;    

    Indices mLeavesOrder;

    NFE_ALIGN(64) std::atomic<uint32> mNumGeneratedNodes;
    NFE_ALIGN(64) std::atomic<uint32> mNumGeneratedLeaves;
};


} // namespace RT
} // namespace NFE
