/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for BVH.
 */

#include "PCH.hpp"
#include "../nfCommon/BVH.hpp"
#include "../nfCommon/Math/Random.hpp"
#include "../nfCommon/Math/Vector.hpp"
#include "../nfCommon/Math/Geometry.hpp"

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

    // TODO
}

TEST(BVH, InsertAndQuery)
{
    const Vector testRegionHalfSize = Vector(100.0f, 100.0f, 100.0f);
    const Vector minBoxHalfSize = Vector(0.5f, 0.5f, 0.5f);
    const Vector maxBoxHalfSize = Vector(3.0f, 3.0f, 3.0f);
    const Vector minQueryHalfSize = Vector(1.0f, 1.0f, 1.0f);
    const Vector maxQueryHalfSize = Vector(30.0f, 30.0f, 30.0f);
    const uint32 numBoxes = 10000;
    const uint32 queriesNum = 500;

    Random random;
    BVH bvh;

    std::vector<uint32> nodeIDs;
    std::vector<Box> boxes;

    for (uint32 i = 0; i < numBoxes; ++i)
    {
        Box box = GenerateRandomBox(random, -testRegionHalfSize, testRegionHalfSize,
                                    minBoxHalfSize, maxBoxHalfSize);

        void* userData = reinterpret_cast<void*>(static_cast<size_t>(i));
        uint32 nodeID = bvh.Insert(box, userData);

        nodeIDs.push_back(nodeID);
        boxes.push_back(box);
    }

    std::vector<uint32> refQueriedLeaves;
    std::vector<uint32> queriedLeaves;
    auto queryCallback = [&] (void* leafUserData)
    {
        uint32 leafID = static_cast<uint32>(reinterpret_cast<size_t>(leafUserData));
        EXPECT_LT(leafID, numBoxes);
        queriedLeaves.push_back(leafID);
    };

    for (uint32 i = 0; i < queriesNum; ++i)
    {
        Box queryBox = GenerateRandomBox(random, -testRegionHalfSize, testRegionHalfSize,
                                         minQueryHalfSize, maxQueryHalfSize);
        SCOPED_TRACE("i = " + std::to_string(i));

        queriedLeaves.clear();
        bvh.Query(queryBox, queryCallback);
        std::sort(queriedLeaves.begin(), queriedLeaves.end());

        refQueriedLeaves.clear();
        for (uint32 j = 0; j < numBoxes; ++j)
        {
            if (Intersect(queryBox, boxes[j]))
                refQueriedLeaves.push_back(j);
        }

        // compare queried leaves with reference
        ASSERT_EQ(refQueriedLeaves.size(), queriedLeaves.size());
        for (size_t j = 0; j < refQueriedLeaves.size(); ++j)
        {
            SCOPED_TRACE("j = " + std::to_string(j));
            EXPECT_EQ(refQueriedLeaves[j], queriedLeaves[j]);
        }
    }
}
