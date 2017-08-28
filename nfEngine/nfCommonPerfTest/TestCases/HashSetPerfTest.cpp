/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for HashSet
 */

#include "PCH.hpp"

#include "nfCommon/nfCommon.hpp"
#include "nfCommon/Containers/HashSet.hpp"
#include "nfCommon/System/Timer.hpp"
#include "nfCommon/Math/Random.hpp"

#include <unordered_set>


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::Math;

TEST(HashSet, Basic)
{
    Timer timer;

    // prepare random data
    const int MAX_NUM_VALUES = 8 * 1024 * 1024;
    std::vector<int> values;
    values.reserve(MAX_NUM_VALUES);

    for (int i = 0; i < MAX_NUM_VALUES; ++i)
        values.push_back(i);

    Math::Random random(0);
    random.ShuffleContainer(values.begin(), values.end(), MAX_NUM_VALUES);


    std::cout << "Values  | Insert (NFE) | Find (NFE)   | Collisions | Insert (std) | Find (std) " << std::endl;
    for (int numValues = 16; numValues <= MAX_NUM_VALUES; numValues *= 2)
    {
        std::cout << std::left << std::setw(10) << numValues;

        // NFE::Common::HashSet
        {
            HashSet<int> set;

            // insert leaves
            timer.Start();
            for (int i = 0; i < numValues; ++i)
            {
                set.Insert(values[i]);
            }
            double insertTime = timer.Stop();
            std::cout << std::setprecision(5) << std::left << std::setw(15) << 1000.0 * insertTime;

            // find leaves
            timer.Start();
            for (int i = 0; i < numValues; ++i)
            {
                auto iter = set.Find(values[i]);
                ASSERT_EQ(values[i], *iter);
            }
            double findTime = timer.Stop();
            std::cout << std::setprecision(5) << std::left << std::setw(15) << 1000.0 * findTime;

            std::cout << std::left << std::setw(14) << set.CalculateHashCollisions();
        }

        // std::unordered_set
        {
            std::unordered_set<int> set;

            // insert leaves
            timer.Start();
            for (int i = 0; i < numValues; ++i)
            {
                set.insert(values[i]);
            }
            double insertTime = timer.Stop();
            std::cout << std::setprecision(5) << std::left << std::setw(15) << 1000.0 * insertTime;

            // find leaves
            timer.Start();
            for (int i = 0; i < numValues; ++i)
            {
                auto iter = set.find(values[i]);
                ASSERT_EQ(values[i], *iter);
            }
            double findTime = timer.Stop();
            std::cout << std::setprecision(5) << std::left << std::setw(15) << 1000.0 * findTime;
        }

        // tree stats
        std::cout << std::endl;
    }
}