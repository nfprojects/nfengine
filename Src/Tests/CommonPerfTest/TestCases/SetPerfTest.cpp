/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for Set
 */

#include "PCH.hpp"

#include "Engine/Common/nfCommon.hpp"
#include "Engine/Common/Containers/Set.hpp"
#include "Engine/Common/System/Timer.hpp"
#include "Engine/Common/Math/Random.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::Math;

TEST(Set, Basic)
{
    Timer timer;

    // prepare random data
    const int MAX_NUM_VALUES = 2 * 1024 * 1024;
    std::vector<int> values;
    values.reserve(MAX_NUM_VALUES);

    for (int i = 0; i < MAX_NUM_VALUES; ++i)
        values.push_back(i);

    Math::Random random;
    random.ShuffleContainer(values.begin(), values.end(), MAX_NUM_VALUES);


    std::cout << "Values  | Insert (NFE) | Find (NFE)   | Insert (std) | Find (std) " << std::endl;
    for (int numValues = 16; numValues <= MAX_NUM_VALUES; numValues *= 2)
    {
        std::cout << std::left << std::setw(10) << numValues;

        // NFE::Common::Set
        {
            Set<int> set;

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
        }

        // std::set
        {
            std::set<int> set;

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