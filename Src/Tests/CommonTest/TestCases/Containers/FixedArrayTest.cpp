/**
 * @file
 * @author Witek902
 * @brief  Unit tests for FixedArray
 */

#include "PCH.hpp"
#include "Engine/Common/Containers/FixedArray.hpp"

#include "TestClasses.hpp"

using namespace NFE;
using namespace NFE::Common;

static const uint32 DefaultSize = 4;

TEST(FixedArray, Default)
{
    FixedArray<int32, DefaultSize> array;

    EXPECT_EQ(DefaultSize, array.Size());
}

TEST(FixedArray, DefaultElement)
{
    const FixedArray<int32, DefaultSize> array(123);

    EXPECT_EQ(DefaultSize, array.Size());
    EXPECT_EQ(123, array[0]);
    EXPECT_EQ(123, array[1]);
    EXPECT_EQ(123, array[2]);
    EXPECT_EQ(123, array[3]);
}

TEST(FixedArray, View)
{
    FixedArray<int32, DefaultSize> array(123);

    EXPECT_EQ(DefaultSize, array.GetView().Size());
}

TEST(FixedArray, InitializerList)
{
    const FixedArray<int32, DefaultSize> array = { 1, 2, 3, 4 };

    EXPECT_EQ(1, array[0]);
    EXPECT_EQ(2, array[1]);
    EXPECT_EQ(3, array[2]);
    EXPECT_EQ(4, array[3]);
}
