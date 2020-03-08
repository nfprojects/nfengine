#include "PCH.h"
#include "BVHBuilder.h"
#include "../Common/System/Timer.hpp"
#include "../Common/Logger/Logger.hpp"
#include "../Common/Utils/TaskBuilder.hpp"
#include "../Common/Utils/Waitable.hpp"
#include "../Common/Utils/ThreadPool.hpp"


namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

void BVHBuilder::ThreadData::Init(uint32 numLeaves)
{
    mLeftBoxesCache.Resize_SkipConstructor(numLeaves);
    mRightBoxesCache.Resize_SkipConstructor(numLeaves);
}

//////////////////////////////////////////////////////////////////////////

BVHBuilder::BVHBuilder(BVH& targetBVH)
    : mLeafBoxes(nullptr)
    , mNumLeaves(0)
    , mNumGeneratedNodes(0)
    , mTarget(targetBVH)
{
}

BVHBuilder::~BVHBuilder()
{

}

bool BVHBuilder::Build(const Box* data, const uint32 numLeaves,
                       const BvhBuildingParams& params,
                       DynArray<uint32>& outLeavesOrder)
{
    mLeafBoxes = data;
    mNumLeaves = numLeaves;
    mParams = params;
    mTarget.AllocateNodes(2 * mNumLeaves);

    mNumGeneratedNodes = 0;
    mNumGeneratedLeaves = 0;
    mLeavesOrder.Clear();
    mLeavesOrder.Resize(mNumLeaves);

    if (mNumLeaves == 0)
    {
        NFE_LOG_INFO("Skipped empty BVH generation");
        return true;
    }

    // calculate overall bounding box
    Box overallBox = Box::Empty();
    for (uint32 i = 0; i < mNumLeaves; ++i)
    {
        overallBox = Box(overallBox, mLeafBoxes[i]);
    }

    NFE_LOG_INFO("BVH statistics: num leaves = %u, overall box = [%f, %f, %f], [%f, %f, %f]",
                mNumLeaves,
                overallBox.min.f[0], overallBox.min.f[1], overallBox.min.f[2],
                overallBox.max.f[0], overallBox.max.f[1], overallBox.max.f[2]);

    WorkSetPtr rootWorkSet = MakeSharedPtr<WorkSet>();
    rootWorkSet->box = overallBox;
    rootWorkSet->numLeaves = mNumLeaves;
    rootWorkSet->leafIndices.Reserve(mNumLeaves);
    for (uint32 i = 0; i < mNumLeaves; ++i)
    {
        rootWorkSet->leafIndices.PushBack(i);
    }

    Timer timer;
    timer.Start();

    uint32 numThreads = ThreadPool::GetInstance().GetNumThreads();
    mThreadData.Resize(numThreads);
    for (uint32 i = 0; i < numThreads; ++i)
    {
        mThreadData[i].Init(mNumLeaves);
    }

    Waitable waitable;
    {
        BVH::Node& rootNode = mTarget.mNodes.Front();
        mNumGeneratedNodes += 2;

        TaskBuilder taskBuilder(waitable);
        taskBuilder.Task("BVHBuilder::Build", [this, rootWorkSet, &rootNode] (const TaskContext& taskContext)
        {
            TaskBuilder childTaskBuilder(taskContext.taskId);
            BuildNode_Threaded(rootWorkSet, rootNode, taskContext, childTaskBuilder);
        });
    }
    waitable.Wait();

    NFE_ASSERT(mNumGeneratedLeaves == mNumLeaves); // Number of generated leaves is invalid
    NFE_ASSERT(mNumGeneratedNodes <= 2 * mNumLeaves); // Number of generated nodes is invalid

    // shrink BVH nodes array
    mTarget.mNumNodes = mNumGeneratedNodes;
    mTarget.mNodes.Resize(mNumGeneratedNodes);
    // mTarget.mNodes.shrink_to_fit(); // TODO

    const float millisecondsElapsed = (float)(1000.0 * timer.Stop());
    NFE_LOG_INFO("Finished BVH generation in %.9g ms (num nodes = %u)", millisecondsElapsed, mNumGeneratedNodes.load());

    outLeavesOrder = mLeavesOrder;
    return true;
}

void BVHBuilder::GenerateLeaf(const WorkSet& workSet, BVH::Node& targetNode)
{
    targetNode.numLeaves = workSet.numLeaves;
    targetNode.childIndex = mNumGeneratedLeaves.fetch_add(workSet.numLeaves);

    for (uint32 i = 0; i < workSet.numLeaves; ++i)
    {
        //mLeavesOrder.PushBack(workSet.leafIndices[i]);
        mLeavesOrder[targetNode.childIndex + i] = workSet.leafIndices[i];
    }
}

void BVHBuilder::SubdivideNode(ThreadData& threadData, const WorkSet& workSet,
    uint32& outAxis, uint32& outSplitPos, Math::Box& outLeftBox, Math::Box& outRightBox) const
{
    uint32 bestAxis = 0;
    uint32 bestSplitPos = 0;
    float bestCost = FLT_MAX;
    Box bestLeftBox = Box::Empty();
    Box bestRightBox = Box::Empty();

    // TODO could be parallelized (only nodes near root)
    for (uint32 axis = 0; axis < NumAxes; ++axis)
    {
        NFE_ASSERT(workSet.sortedLeavesIndicesCache[axis].Size() == workSet.numLeaves);

        const uint32* sortedIndices = workSet.sortedLeavesIndicesCache[axis].Data();

        // calculate left child node AABB for each possible split position
        {
            Box accumulatedBox = Box::Empty();
            for (uint32 i = 0; i < workSet.numLeaves; ++i)
            {
                accumulatedBox = Box(accumulatedBox, mLeafBoxes[sortedIndices[i]]);
                threadData.mLeftBoxesCache[i] = accumulatedBox;
            }
        }

        // calculate right child node AABB for each possible split position
        {
            Box accumulatedBox = Box::Empty();
            for (uint32 i = workSet.numLeaves; i-- > 0; )
            {
                accumulatedBox = Box(accumulatedBox, mLeafBoxes[sortedIndices[i]]);
                threadData.mRightBoxesCache[i] = accumulatedBox;
            }
        }

        // find optimal split position (surface area heuristics)
        for (uint32 splitPos = 0; splitPos < workSet.numLeaves - 1; ++splitPos)
        {
            const Box& leftBox = threadData.mLeftBoxesCache[splitPos];
            const Box& rightBox = threadData.mRightBoxesCache[splitPos + 1];

            float leftCost = 0.0f, rightCost = 0.0f;
            if (mParams.heuristics == BvhBuildingParams::Heuristics::SurfaceArea)
            {
                leftCost = leftBox.SurfaceArea();
                rightCost = rightBox.SurfaceArea();
            }
            else if (mParams.heuristics == BvhBuildingParams::Heuristics::Volume)
            {
                leftCost = leftBox.Volume();
                rightCost = rightBox.Volume();
            }
            else
            {
                NFE_FATAL();
            }

            const uint32 leftCount = splitPos + 1;
            const uint32 rightCount = workSet.numLeaves - leftCount;
            const float totalCost = leftCost * static_cast<float>(leftCount) + rightCost * static_cast<float>(rightCount);

            if (totalCost < bestCost)
            {
                bestCost = totalCost;
                bestAxis = axis;
                bestSplitPos = splitPos;
                bestLeftBox = leftBox;
                bestRightBox = rightBox;
            }
        }
    }

    outAxis = bestAxis;
    outSplitPos = bestSplitPos;
    outLeftBox = bestLeftBox;
    outRightBox = bestRightBox;
}

void BVHBuilder::BuildNode(ThreadData& threadData, WorkSet& workSet, BVH::Node& targetNode)
{
    NFE_ASSERT(workSet.numLeaves <= mNumLeaves);
    NFE_ASSERT(workSet.numLeaves > 0);
    NFE_ASSERT(workSet.depth < mNumLeaves);
    NFE_ASSERT(workSet.depth <= BVH::MaxDepth);

    targetNode.min = workSet.box.min.ToFloat3();
    targetNode.max = workSet.box.max.ToFloat3();

    if (workSet.numLeaves <= mParams.maxLeafNodeSize)
    {
        GenerateLeaf(workSet, targetNode);
        return;
    }

    SortLeaves(workSet);

    uint32 bestAxis = 0;
    uint32 bestSplitPos = 0;
    Box bestLeftBox = Box::Empty();
    Box bestRightBox = Box::Empty();
    SubdivideNode(threadData, workSet, bestAxis, bestSplitPos, bestLeftBox, bestRightBox);

    const uint32 leftCount = bestSplitPos + 1;
    const uint32 rightCount = workSet.numLeaves - leftCount;

    const uint32 leftNodeIndex = mNumGeneratedNodes.fetch_add(2);

    targetNode.childIndex = leftNodeIndex;
    targetNode.numLeaves = 0;
    targetNode.splitAxis = bestAxis;

    const Indices& sortedIndices = workSet.sortedLeavesIndicesCache[bestAxis];

    Indices leftIndices, rightIndices;
    leftIndices.Resize_SkipConstructor(leftCount);
    rightIndices.Resize_SkipConstructor(rightCount);
    memcpy(leftIndices.Data(), sortedIndices.Data(), sizeof(uint32) * leftCount);
    memcpy(rightIndices.Data(), sortedIndices.Data() + leftCount, sizeof(uint32) * rightCount);

    WorkSet childWorkSet;
    childWorkSet.sortedBy = bestAxis;
    childWorkSet.depth = workSet.depth + 1;

    // generate left node
    {
        BVH::Node& childNode = mTarget.mNodes[leftNodeIndex];

        childWorkSet.box = bestLeftBox;
        childWorkSet.numLeaves = leftCount;
        childWorkSet.leafIndices = std::move(leftIndices);
        BuildNode(threadData, childWorkSet, childNode);
    }

    // generate right node
    {
        BVH::Node& childNode = mTarget.mNodes[leftNodeIndex + 1];

        childWorkSet.box = bestRightBox;
        childWorkSet.numLeaves = rightCount;
        childWorkSet.leafIndices = std::move(rightIndices);
        BuildNode(threadData, childWorkSet, childNode);
    }
}

void BVHBuilder::BuildNode_Threaded(const WorkSetPtr& workSet, BVH::Node& targetNode, const TaskContext& taskContext, TaskBuilder& taskBuilder)
{
    if (workSet->numLeaves < 2000)
    {
        ThreadData& threadData = mThreadData[taskContext.threadId];
        BuildNode(threadData , *workSet, targetNode);
        return;
    }

    NFE_ASSERT(workSet->numLeaves <= mNumLeaves);
    NFE_ASSERT(workSet->numLeaves > 0);
    NFE_ASSERT(workSet->depth < mNumLeaves);
    NFE_ASSERT(workSet->depth <= BVH::MaxDepth);

    targetNode.min = workSet->box.min.ToFloat3();
    targetNode.max = workSet->box.max.ToFloat3();

    if (workSet->numLeaves <= mParams.maxLeafNodeSize)
    {
        GenerateLeaf(*workSet, targetNode);
        return;
    }

    SortLeaves_Threaded(workSet, taskBuilder);

    taskBuilder.Fence();

    taskBuilder.Task("BVHBuilder::BuildNode", [this, workSet, &targetNode] (const TaskContext& taskContext)
    {
        ThreadData& threadData = mThreadData[taskContext.threadId];

        uint32 bestAxis = 0;
        uint32 bestSplitPos = 0;
        Box bestLeftBox = Box::Empty();
        Box bestRightBox = Box::Empty();
        SubdivideNode(threadData, *workSet, bestAxis, bestSplitPos, bestLeftBox, bestRightBox);

        const uint32 leftCount = bestSplitPos + 1;
        const uint32 rightCount = workSet->numLeaves - leftCount;

        const uint32 leftNodeIndex = mNumGeneratedNodes.fetch_add(2);

        targetNode.childIndex = leftNodeIndex;
        targetNode.numLeaves = 0;
        targetNode.splitAxis = bestAxis;

        const Indices& sortedIndices = workSet->sortedLeavesIndicesCache[bestAxis];

        Indices leftIndices, rightIndices;
        leftIndices.Resize_SkipConstructor(leftCount);
        rightIndices.Resize_SkipConstructor(rightCount);
        memcpy(leftIndices.Data(), sortedIndices.Data(), sizeof(uint32) * leftCount);
        memcpy(rightIndices.Data(), sortedIndices.Data() + leftCount, sizeof(uint32) * rightCount);

        // generate left node
        {
            TaskBuilder taskBuilder(taskContext.taskId);

            BVH::Node& childNode = mTarget.mNodes[leftNodeIndex];

            WorkSetPtr childWorkSet = MakeSharedPtr<WorkSet>();
            childWorkSet->sortedBy = bestAxis;
            childWorkSet->depth = workSet->depth + 1;
            childWorkSet->box = bestLeftBox;
            childWorkSet->numLeaves = leftCount;
            childWorkSet->leafIndices = std::move(leftIndices);
            BuildNode_Threaded(childWorkSet, childNode, taskContext, taskBuilder);
        }

        // generate right node
        {
            TaskBuilder taskBuilder(taskContext.taskId);

            BVH::Node& childNode = mTarget.mNodes[leftNodeIndex + 1];

            WorkSetPtr childWorkSet = MakeSharedPtr<WorkSet>();
            childWorkSet->sortedBy = bestAxis;
            childWorkSet->depth = workSet->depth + 1;
            childWorkSet->box = bestRightBox;
            childWorkSet->numLeaves = rightCount;
            childWorkSet->leafIndices = std::move(rightIndices);
            BuildNode_Threaded(childWorkSet, childNode, taskContext, taskBuilder);
        }
    });
}

void BVHBuilder::SortLeavesInAxis(Indices& indicesToSort, uint32 axis) const
{
    const auto comparator = [this, axis] (const uint32 a, const uint32 b)
    {
        const Box& leafA = mLeafBoxes[a];
        const Box& leafB = mLeafBoxes[b];
        const Vector4 centerA = leafA.max + leafA.min;
        const Vector4 centerB = leafB.max + leafB.min;
        return centerA[axis] < centerB[axis];
    };

    std::sort(indicesToSort.begin(), indicesToSort.end(), comparator);
}

void BVHBuilder::SortLeaves(WorkSet& workSet) const
{
    NFE_ASSERT(workSet.leafIndices.Size() == workSet.numLeaves);

    for (uint32 axis = 0; axis < NumAxes; ++axis)
    {
        Indices& indicesToSort = workSet.sortedLeavesIndicesCache[axis];

        if (workSet.sortedBy != axis) // sort only what needs to be sorted
        {
            // copy from unsorted
            indicesToSort = workSet.leafIndices;

            SortLeavesInAxis(indicesToSort, axis);
        }
    }

    if (workSet.sortedBy < NumAxes)
    {
        workSet.sortedLeavesIndicesCache[workSet.sortedBy] = std::move(workSet.leafIndices);
    }
}

void BVHBuilder::SortLeaves_Threaded(const WorkSetPtr& workSet, TaskBuilder& taskBuilder) const
{
    NFE_ASSERT(workSet->leafIndices.Size() == workSet->numLeaves);

    for (uint32 axis = 0; axis < NumAxes; ++axis)
    {
        Indices& indicesToSort = workSet->sortedLeavesIndicesCache[axis];

        if (workSet->sortedBy != axis) // sort only what needs to be sorted
        {
            // copy from unsorted
            indicesToSort = workSet->leafIndices;

            if (indicesToSort.Size() > 10000)
            {
                taskBuilder.Task("BVHBuilder::SortLeaves/Axis", [axis, workSet, this] (const TaskContext&)
                {
                    Indices& indicesToSort = workSet->sortedLeavesIndicesCache[axis];
                    SortLeavesInAxis(indicesToSort, axis);
                });
            }
            else
            {
                SortLeavesInAxis(indicesToSort, axis);
            }
        }
    }

    if (workSet->sortedBy < NumAxes)
    {
        workSet->sortedLeavesIndicesCache[workSet->sortedBy] = std::move(workSet->leafIndices);
    }
}

} // namespace RT
} // namespace NFE
