#include "PCH.hpp"
#include "nfCommon/Utils/Entropy.hpp"
#include "nfCommon/Math/Random.hpp"

using namespace NFE;
using namespace NFE::Math;

//////////////////////////////////////////////////////////////////////////

TEST(RandomTest, Entropy)
{
    Common::Entropy entropy;

    const uint32 iterations = 10000;

    uint64 sum = 0;
    for (uint32 i = 0; i < iterations; ++i)
    {
        sum += entropy.GetInt();
    }

    EXPECT_GE(sum, 4900ull * (uint64)UINT32_MAX);
    EXPECT_LE(sum, 5100ull * (uint64)UINT32_MAX);
}
