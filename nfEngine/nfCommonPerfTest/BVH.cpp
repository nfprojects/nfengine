/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for Bounding Volume Hierarchy
 */

#include "PCH.hpp"

#include "../nfCommon/BVH.hpp"
#include "../nfCommon/Timer.hpp"
#include "../nfCommon/Math/Random.hpp"
#include "../nfCommon/Math/Vector.hpp"

using namespace NFE::Common;
using namespace NFE::Math;

/**
 * 1. Insert random boxes.
 * 2. Perform queries.
 * 3. Remove all leaves.
 */
TEST(BVH, InsertQueryAndRemove)
{
    Timer timer;
    Random random;

    const int leavesNumStart = 32;
    const int leavesNumEnd = 2000000;
    const int queriesNum = 1024;
    const Vector boxHalfSize = Vector(0.5f, 0.5f, 0.5f);

    std::vector<BVHStats> statsArray;

    auto queryCallback = [] (void* leafUserData)
    {
        (void)leafUserData;
    };

    std::cout << "Leaves   | Insert time [ms] | Query time [us] | Big query time [us] "
                 "| Remove time [ms]" << std::endl;
    for (int leavesNum = leavesNumStart; leavesNum < leavesNumEnd; leavesNum *= 2)
    {
        std::cout << std::left << std::setw(8) << leavesNum << "   ";

        BVH bvh;
        std::vector<uint32> nodeIDs;

        // insert leaves
        timer.Start();
        for (int i = 0; i < leavesNum; ++i)
        {
            Vector center(random.GetFloat3());
            center *= 100.0f;

            Box box(center - boxHalfSize, center + boxHalfSize);
            nodeIDs.push_back(bvh.Insert(box, nullptr));
        }
        double insertTime = timer.Stop();
        std::cout << std::setprecision(4) << std::left << std::setw(16) <<
            1000.0 * insertTime << "   ";


        // perform small queries
        timer.Start();
        for (int i = 0; i < queriesNum; ++i)
        {
            Vector center(random.GetFloat3());
            center *= 100.0f;
            Box box(center - boxHalfSize, center + boxHalfSize);

            bvh.Query(box, queryCallback);
        }
        double queryTime = timer.Stop();
        std::cout << std::setprecision(6) << std::left << std::setw(15) <<
            1000000.0 * queryTime / static_cast<double>(queriesNum) << "   ";


        // perform big queries
        timer.Start();
        for (int i = 0; i < queriesNum; ++i)
        {
            Vector center(random.GetFloat3());
            center *= 100.0f;
            Box box(center - boxHalfSize * 20.0f, center + boxHalfSize * 20.0f);

            bvh.Query(box, queryCallback);
        }
        double bigQueryTime = timer.Stop();
        std::cout << std::setprecision(6) << std::left << std::setw(19) <<
            1000000.0 * bigQueryTime / static_cast<double>(queriesNum) << "   ";


        // record stats
        BVHStats stats;
        bvh.GetStats(&stats);
        statsArray.push_back(stats);

        // remove leaves
        timer.Start();
        for (int i = 0; i < leavesNum; ++i)
            bvh.Remove(nodeIDs[i]);
        double removeTime = timer.Stop();
        std::cout << std::setprecision(4) << std::left << std::setw(16) <<
            1000.0 * removeTime << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Leaves   | Tree height | Total area  | Total volume" << std::endl;
    int leavesNum = leavesNumStart;
    for (size_t i = 0; i < statsArray.size(); ++i)
    {
        std::cout << std::left << std::setw(8) << leavesNum << "   ";
        std::cout << std::setprecision(5) << std::left <<
            std::setw(11) << statsArray[i].height << "   " <<
            std::setw(11) << statsArray[i].totalArea << "   " <<
            std::setw(11) << statsArray[i].totalVolume << std::endl;

            leavesNum *= 2;
    }
}

TEST(BVH, TreeBalance)
{
    Timer timer;
    Random random;

    const uint32 iterations = 32;
    const uint32 initialLeaves = 100000;
    const uint32 leavesToReplace = 20000;
    const uint32 queriesNum = 500;
    const Vector boxHalfSize = Vector(0.5f, 0.5f, 0.5f);

    BVH bvh;
    std::vector<BVHStats> statsArray;
    std::vector<uint32> nodeIDs;
    std::vector<uint32> indicies; // for shuffling

    auto queryCallback = [] (void* leafUserData)
    {
        (void)leafUserData;
    };

    // insert initial leaves
    std::cout << "Inserting initial leaves..." << std::endl;
    for (uint32 i = 0; i < initialLeaves; ++i)
    {
        Vector center(random.GetFloat3());
        center *= 100.0f;

        Box box(center - boxHalfSize, center + boxHalfSize);
        nodeIDs.push_back(bvh.Insert(box, nullptr));
        indicies.push_back(i);
    }

    // record stats of the initial tree
    BVHStats stats;
    bvh.GetStats(&stats);
    statsArray.push_back(stats);

    // run test iterations
    std::cout << "Iter. | Query time [us] | Big query time [us] "
        "| Remove time [ms] | Insert time [ms]" << std::endl;
    for (int iteration = 0; iteration < iterations; ++iteration)
    {
        std::cout << std::left << std::setw(5) << iteration << "   ";

        // perform small queries
        timer.Start();
        for (uint32 i = 0; i < queriesNum; ++i)
        {
            Vector center(random.GetFloat3());
            center *= 100.0f;
            Box box(center - boxHalfSize, center + boxHalfSize);

            bvh.Query(box, queryCallback);
        }
        double queryTime = timer.Stop();
        std::cout << std::setprecision(6) << std::left << std::setw(15) <<
            1000000.0 * queryTime / static_cast<double>(queriesNum) << "   ";

        // perform big queries
        timer.Start();
        for (uint32 i = 0; i < queriesNum; ++i)
        {
            Vector center(random.GetFloat3());
            center *= 100.0f;
            Box box(center - boxHalfSize * 20.0f, center + boxHalfSize * 20.0f);

            bvh.Query(box, queryCallback);
        }
        double bigQueryTime = timer.Stop();
        std::cout << std::setprecision(6) << std::left << std::setw(19) <<
            1000000.0 * bigQueryTime / static_cast<double>(queriesNum) << "   ";


        random.ShuffleContainer(indicies.begin(), indicies.end(), leavesToReplace);

        // remove leaves
        timer.Start();
        for (uint32 i = 0; i < leavesToReplace; ++i)
            bvh.Remove(nodeIDs[indicies[i]]);
        double removeTime = timer.Stop();
        std::cout << std::setprecision(4) << std::left << std::setw(16) <<
            1000.0 * removeTime << "  ";

        // insert new leaves in place of the removed
        timer.Start();
        for (int i = 0; i < leavesToReplace; ++i)
        {
            Vector center(random.GetFloat3());
            center *= 100.0f;

            Box box(center - boxHalfSize, center + boxHalfSize);
            nodeIDs[indicies[i]] = bvh.Insert(box, nullptr);
        }
        double insertTime = timer.Stop();
        std::cout << std::setprecision(4) << std::left << std::setw(16) <<
            1000.0 * insertTime << std::endl;

        ASSERT_EQ(initialLeaves, bvh.GetSize());

        // record stats
        bvh.GetStats(&stats);
        statsArray.push_back(stats);
    }

    std::cout << std::endl;
    std::cout << "Iter. | Tree height | Total area  | Total volume" << std::endl;
    for (size_t i = 0; i < statsArray.size(); ++i)
    {
        std::cout << std::left << std::setw(5) << i << "   ";
        std::cout << std::setprecision(5) << std::left <<
            std::setw(11) << statsArray[i].height << "   " <<
            std::setw(11) << statsArray[i].totalArea << "   " <<
            std::setw(11) << statsArray[i].totalVolume << std::endl;
    }
}

// TODO: mixed insertions, remoals and queries
