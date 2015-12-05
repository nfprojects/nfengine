/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for ThreadPool class.
 */

#include "PCH.hpp"
#include "../nfCommon/BVH.hpp"
#include "../nfCommon/Timer.hpp"
#include "../nfCommon/Math/Random.hpp"
#include "../nfCommon/Math/Vector.hpp"

using namespace NFE::Common;
using namespace NFE::Math;

TEST(BVH, InsertAndRemove)
{
    Timer timer;
    Random random;

    const int leavesNumStart = 32;
    const int leavesNumEnd = 2'000'000;
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
            Box box(center - boxHalfSize * 20.0f, center + boxHalfSize* 20.0f);

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

    // TEMPORARY
    getchar();
}
