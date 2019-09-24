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

namespace {

volatile int gVolatileTempValue = 0;

void HashSetPerfTest(size_t setSize, const std::vector<int>& values)
{
    Timer timer;
    HashSet<int> set;

    // insert keys
    {
        timer.Start();
        for (size_t i = 0; i < setSize; ++i)
        {
            set.Insert(values[i]);
        }
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }

    // find keys
    {
        timer.Start();
        for (size_t i = 0; i < setSize; ++i)
        {
            const auto iter = set.Find(values[i]);
            ASSERT_EQ(values[i], *iter);
        }
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }

    // iterate the whole set
    {
        timer.Start();
        int sum = 0;
        for (int val : set)
        {
            sum += val;
        }
        gVolatileTempValue = sum; // force the compiler to not optimize the loop out
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }

    // erase some keys
    {
        timer.Start();
        for (size_t i = 0; i < setSize * 2 / 3; ++i)
        {
            set.Erase(values[i]);
        }
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }


    // iterate the whole set again
    {
        timer.Start();
        int sum = 0;
        for (int val : set)
        {
            sum += val;
        }
        gVolatileTempValue = sum; // force the compiler to not optimize the loop out
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }

    std::cout << std::left << std::setw(13) << set.CalculateHashCollisions();
}

void StlUnorderedSetPerfTest(size_t setSize, const std::vector<int>& values)
{
    Timer timer;
    std::unordered_set<int> set;

    // insert keys
    {
        timer.Start();
        for (size_t i = 0; i < setSize; ++i)
        {
            set.insert(values[i]);
        }
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }

    // find keys
    {
        timer.Start();
        for (size_t i = 0; i < setSize; ++i)
        {
            const auto iter = set.find(values[i]);
            ASSERT_EQ(values[i], *iter);
        }
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }

    // iterate the whole set
    {
        timer.Start();
        int sum = 0;
        for (int val : set)
        {
            sum += val;
        }
        gVolatileTempValue = sum; // force the compiler to not optimize the loop out
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }

    // erase some keys
    {
        timer.Start();
        for (size_t i = 0; i < setSize * 2 / 3; ++i)
        {
            set.erase(values[i]);
        }
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }

    // iterate the whole again
    {
        timer.Start();
        int sum = 0;
        for (int val : set)
        {
            sum += val;
        }
        gVolatileTempValue = sum; // force the compiler to not optimize the loop out
        double t = timer.Stop();
        std::cout << std::setprecision(3) << std::left << std::setw(10) << (1000.0 * t);
    }
}

} // namespace


TEST(HashSet, Basic)
{
    Timer timer;

    // prepare random data
    const size_t minValues = 64;
    const size_t maxValues = 16 * 1024 * 1024;
    std::vector<int> values;
    std::vector<int> valuesToErase;
    {
        values.reserve(maxValues);

        Math::Random random;
        for (size_t i = 0; i < maxValues; ++i)
        {
            values.push_back(random.GetInt());
        }

        random.ShuffleContainer(values.begin(), values.end(), maxValues);
    }

    std::cout
        << "1 - inserting [ms]" << std::endl
        << "2 - finding [ms]" << std::endl
        << "3 - iterating [ms]" << std::endl
        << "4 - erasing [ms]" << std::endl
        << "C - number of hash collisions" << std::endl
        << "---------------------------------------------------------------------" << std::endl
        << "Num keys | 1       | 2       | 3       | 4       | 5       | C" << std::endl
        << "---------------------------------------------------------------------" << std::endl
        << "NFE::Common::HashSet" << std::endl
        << "---------------------------------------------------------------------" << std::endl;

    for (size_t numValues = minValues; numValues <= maxValues; numValues *= 2)
    {
        std::cout << std::left << std::setw(11) << numValues;
        HashSetPerfTest(numValues, values);
        std::cout << std::endl;
    }

    std::cout
        << "---------------------------------------------------------------------" << std::endl
        << "std::unordered_set " << std::endl
        << "---------------------------------------------------------------------" << std::endl;

    for (size_t numValues = minValues; numValues <= maxValues; numValues *= 2)
    {
        std::cout << std::left << std::setw(11) << numValues;
        StlUnorderedSetPerfTest(numValues, values);
        std::cout << std::endl;
    }
}