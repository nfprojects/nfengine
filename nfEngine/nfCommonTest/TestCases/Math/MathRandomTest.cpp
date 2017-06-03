#include "PCH.hpp"
#include "nfCommon/Math/Random.hpp"

using namespace NFE;
using namespace NFE::Math;

TEST(Math, Random_SameSeed)
{
    const uint64 seed = 12345;
    Random randomA(seed);
    Random randomB(seed);

    ASSERT_EQ(randomA.GetInt(), randomB.GetInt());
}

TEST(Math, Random_DifferentSeed)
{
    Random randomA(12345);
    Random randomB(67890);

    ASSERT_NE(randomA.GetInt(), randomB.GetInt());
}

