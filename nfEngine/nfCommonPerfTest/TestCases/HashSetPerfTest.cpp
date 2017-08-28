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
    const int MAX_NUM_VALUES = 16 * 1024 * 1024;
    std::vector<int> values;
    {
        values.reserve(MAX_NUM_VALUES);

        Math::Random random(0);
        for (int i = 0; i < MAX_NUM_VALUES; ++i)
        {
            values.push_back(random.GetInt());
        }

        random.ShuffleContainer(values.begin(), values.end(), MAX_NUM_VALUES);
    }

    std::cout << "         | NFE::Common::HashSet                  | std::unordered_set " << std::endl;
    std::cout << "Num keys | Insert [ms] | Find [ms]  | Collisions | Insert [ms] | Find [ms]" << std::endl;
    std::cout << "-----------------------------------------------------------------------------" << std::endl;
    for (int numValues = 16; numValues <= MAX_NUM_VALUES; numValues *= 2)
    {
        std::cout << std::left << std::setw(11) << numValues;

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
            std::cout << std::setprecision(4) << std::left << std::setw(14) << 1000.0 * insertTime;

            // find leaves
            timer.Start();
            for (int i = 0; i < numValues; ++i)
            {
                auto iter = set.Find(values[i]);
                ASSERT_EQ(values[i], *iter);
            }
            double findTime = timer.Stop();
            std::cout << std::setprecision(4) << std::left << std::setw(12) << 1000.0 * findTime;

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
            std::cout << std::setprecision(4) << std::left << std::setw(14) << 1000.0 * insertTime;

            // find leaves
            timer.Start();
            for (int i = 0; i < numValues; ++i)
            {
                auto iter = set.find(values[i]);
                ASSERT_EQ(values[i], *iter);
            }
            double findTime = timer.Stop();
            std::cout << std::setprecision(4) << std::left << std::setw(12) << 1000.0 * findTime;
        }

        // tree stats
        std::cout << std::endl;
    }
}