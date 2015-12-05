/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for BVH.
 */

#include "PCH.hpp"

#include "Common.hpp"

#include "../nfCommon/BVH.hpp"
#include "../nfCommon/Math/Random.hpp"
#include "../nfCommon/Math/Vector.hpp"
#include "../nfCommon/Math/Geometry.hpp"
#include "../nfCommon/Logger.hpp"


using namespace NFE::Common;
using namespace NFE::Math;

// Generate a random box with a given range of center position and range of half size
Box GenerateRandomBox(Random& random,
                      const Vector& minCenter, const Vector& maxCenter,
                      const Vector& minHalfSize, const Vector& maxHalfSize)
{
    Vector center = minCenter + (maxCenter - minCenter) * Vector(random.GetFloat3());
    Vector halfSize = minHalfSize + (maxHalfSize - minHalfSize) * Vector(random.GetFloat3());
    return Box(center - halfSize, center + halfSize);
}

TEST(BVH, Simple)
{
    BVH bvh;
    const uint32 invalidLeaf = 0x12345678;

    // insert two test leaves
    EXPECT_EQ(0, bvh.GetSize());
    uint32 leaf0 = bvh.Insert(Box(), nullptr);
    EXPECT_EQ(1, bvh.GetSize());
    uint32 leaf1 = bvh.Insert(Box(), nullptr);
    EXPECT_EQ(2, bvh.GetSize());
    ASSERT_NE(leaf0, leaf1);

    // move tests
    EXPECT_TRUE(bvh.Move(leaf0, Box()));
    EXPECT_TRUE(bvh.Move(leaf1, Box()));
    EXPECT_FALSE(bvh.Move(invalidLeaf, Box()));

    // remove test
    EXPECT_TRUE(bvh.Remove(leaf0));
    EXPECT_EQ(1, bvh.GetSize());
    EXPECT_TRUE(bvh.Remove(leaf1));
    EXPECT_EQ(0, bvh.GetSize());

    // try to remove invalid leaves
    EXPECT_FALSE(bvh.Remove(invalidLeaf));
    EXPECT_FALSE(bvh.Remove(leaf0));
    EXPECT_FALSE(bvh.Remove(leaf1));
    EXPECT_EQ(0, bvh.GetSize());
}

/**
 * This test works as follows:
 *
 * 1. Iinitial leaves are generated (random boxes).
 * 2. In each iteration:
 * 2.1. Perform BVH query with random query box.
 * 2.2. Generate reference (expected leaves list generated by the query in 2.1).
 * 2.3. Compare results of 2.1 and 2.2.
 * 2.4. Remove random leaves.
 * 2.5. Insert new boxes in place of the removed ones in 2.4.
 * 2.6. Insert more new boxes (so the total list of leaves in the BVH increases with each
        iteration).
 */
TEST(BVH, QueryInsertRemove)
{
    const Vector testRegionHalfSize = Vector(50.0f, 50.0f, 50.0f);
    const Vector minBoxHalfSize = Vector(0.5f, 0.5f, 0.5f);
    const Vector maxBoxHalfSize = Vector(3.0f, 3.0f, 3.0f);
    const Vector minQueryHalfSize = Vector(1.0f, 1.0f, 1.0f);
    const Vector maxQueryHalfSize = Vector(20.0f, 20.0f, 20.0f);

    const uint32 initialLeavesCount = 500;
    const uint32 leavesToBeRemoved = 250;  // number of leaves to be removed in each iteration
    const uint32 leavesToBeInserted = 500; // number of new leaves to be inserted in each iteration
    const uint32 queriesNum = 200;         // number of BVH queries to be performed in each iteration

    static_assert(initialLeavesCount >= leavesToBeRemoved, "Invalid test parameters");
    static_assert(leavesToBeInserted >= leavesToBeRemoved, "Invalid test parameters");

    Random random;
    BVH bvh;

    uint32 totalQueries = 0;
    std::vector<uint32> shuffledIndicies;
    std::vector<uint32> nodeIDs;
    std::vector<Box> boxes;

    // step 1: generate initial leaves
    for (uint32 i = 0; i < initialLeavesCount; ++i)
    {
        Box box = GenerateRandomBox(random, -testRegionHalfSize, testRegionHalfSize,
                                    minBoxHalfSize, maxBoxHalfSize);

        void* userData = reinterpret_cast<void*>(static_cast<size_t>(i));
        uint32 nodeID = bvh.Insert(box, userData);

        shuffledIndicies.push_back(i);
        nodeIDs.push_back(nodeID);
        boxes.push_back(box);
    }
    ASSERT_EQ(initialLeavesCount, bvh.GetSize());

    std::vector<uint32> refQueriedLeaves;
    std::vector<uint32> queriedLeaves;
    auto queryCallback = [&] (void* leafUserData)
    {
        uint32 leavesNum = static_cast<uint32>(nodeIDs.size());
        uint32 leafID = static_cast<uint32>(reinterpret_cast<size_t>(leafUserData));
        EXPECT_LT(leafID, leavesNum);
        queriedLeaves.push_back(leafID);
        totalQueries++;
    };

    // step 2:
    for (uint32 i = 0; i < queriesNum; ++i)
    {
        SCOPED_TRACE("i = " + std::to_string(i));
        uint32 leavesNum = static_cast<uint32>(nodeIDs.size());
        Box queryBox = GenerateRandomBox(random, -testRegionHalfSize, testRegionHalfSize,
                                         minQueryHalfSize, maxQueryHalfSize);

        // step 2.1: perform BVH query (fast)
        queriedLeaves.clear();
        bvh.Query(queryBox, queryCallback);
        std::sort(queriedLeaves.begin(), queriedLeaves.end());

        // step 2.2: create reference list (slow)
        refQueriedLeaves.clear();
        for (uint32 j = 0; j < leavesNum; ++j)
            if (Intersect(queryBox, boxes[j]))
                refQueriedLeaves.push_back(j);

        // step 2.3: compare queried leaves with reference
        ASSERT_EQ(refQueriedLeaves.size(), queriedLeaves.size());
        for (size_t j = 0; j < refQueriedLeaves.size(); ++j)
        {
            SCOPED_TRACE("j = " + std::to_string(j));
            EXPECT_EQ(refQueriedLeaves[j], queriedLeaves[j]);
        }


        // step 2.4: generate list of random leaves to be removed
        random_unique(shuffledIndicies.begin(), shuffledIndicies.end(), leavesToBeRemoved);
        for (uint32 j = 0; j < leavesToBeRemoved; ++j)
        {
            uint32 id = nodeIDs[shuffledIndicies[j]];
            SCOPED_TRACE("j = " + std::to_string(j) + ", id = " + std::to_string(id));
            ASSERT_TRUE(bvh.Remove(id));
        }
        ASSERT_EQ(leavesNum - leavesToBeRemoved, bvh.GetSize());

        // step 2.5: insert new boxes in place of the removed
        for (uint32 j = 0; j < leavesToBeRemoved; ++j)
        {
            SCOPED_TRACE("j = " + std::to_string(j));
            Box box = GenerateRandomBox(random, -testRegionHalfSize, testRegionHalfSize,
                                        minBoxHalfSize, maxBoxHalfSize);

            void* userData = reinterpret_cast<void*>(static_cast<size_t>(shuffledIndicies[j]));
            uint32 nodeID = bvh.Insert(box, userData);

            nodeIDs[shuffledIndicies[j]] = nodeID;
            boxes[shuffledIndicies[j]] = box;
        }
        ASSERT_EQ(leavesNum, bvh.GetSize());

        // step 2.6: insert new boxes
        for (uint32 j = leavesNum; j < leavesNum - leavesToBeRemoved + leavesToBeInserted; ++j)
        {
            Box box = GenerateRandomBox(random, -testRegionHalfSize, testRegionHalfSize,
                                        minBoxHalfSize, maxBoxHalfSize);

            void* userData = reinterpret_cast<void*>(static_cast<size_t>(j));
            uint32 nodeID = bvh.Insert(box, userData);

            shuffledIndicies.push_back(j);
            nodeIDs.push_back(nodeID);
            boxes.push_back(box);
        }
        ASSERT_EQ(leavesNum - leavesToBeRemoved + leavesToBeInserted, bvh.GetSize());
    }

    LOG_INFO("Total queries: %u", totalQueries);
}
