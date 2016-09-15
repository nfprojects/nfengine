/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Set
 */

#include "PCH.hpp"
#include "nfCommon/Set.hpp"
#include "nfCommon/Math/Random.hpp"

using namespace NFE;
using namespace NFE::Common;

TEST(Set, Empty)
{
    Set<int> set;

    EXPECT_EQ(0, set.Size());
    EXPECT_TRUE(set.Empty());
    EXPECT_EQ(set.Begin(), set.End());
    EXPECT_EQ(set.End(), set.Find(0));
    EXPECT_FALSE(set.Erase(0));
    EXPECT_FALSE(set.Erase(set.End()));
}

TEST(Set, SimpleInsert)
{
    Set<int> set;

    ASSERT_NE(set.End(), set.Insert(1).iterator);
    // set = [1]
    EXPECT_EQ(1, set.Size());
    EXPECT_FALSE(set.Empty());

    ASSERT_NE(set.End(), set.Insert(2).iterator);
    // set = [1,2]
    EXPECT_EQ(2, set.Size());
    EXPECT_FALSE(set.Empty());
}

TEST(Set, Iterator)
{
    Set<int> set;
    ASSERT_NE(set.End(), set.Insert(1).iterator);
    ASSERT_NE(set.End(), set.Insert(4).iterator);
    ASSERT_NE(set.End(), set.Insert(2).iterator);
    ASSERT_NE(set.End(), set.Insert(3).iterator);
    ASSERT_NE(set.End(), set.Insert(0).iterator);
    EXPECT_EQ(5, set.Size());

    auto iter = set.Begin();
    EXPECT_EQ(0, *iter);

    ++iter;
    ASSERT_NE(set.Begin(), iter);
    ASSERT_NE(set.End(), iter);
    EXPECT_EQ(1, *iter);

    ++iter;
    ASSERT_NE(set.Begin(), iter);
    ASSERT_NE(set.End(), iter);
    EXPECT_EQ(2, *iter);

    ++iter;
    ASSERT_NE(set.Begin(), iter);
    ASSERT_NE(set.End(), iter);
    EXPECT_EQ(3, *iter);

    ++iter;
    ASSERT_NE(set.Begin(), iter);
    ASSERT_NE(set.End(), iter);
    EXPECT_EQ(4, *iter);

    ++iter;
    ASSERT_NE(set.Begin(), iter);
    ASSERT_EQ(set.End(), iter);
}

TEST(Set, Find)
{
    const int size = 10;
    Set<int> set;

    for (int i = 0; i < size; ++i)
        ASSERT_NE(set.End(), set.Insert(i).iterator);

    for (int i = 0; i < size; ++i)
    {
        auto iter = set.Find(i);
        ASSERT_NE(set.End(), iter);
        EXPECT_EQ(i, *iter);
    }

    EXPECT_EQ(set.End(), set.Find(1234));
    EXPECT_EQ(set.End(), set.Find(-1234));
}

TEST(Set, RangeBasedFor)
{
    Set<int> set;
    ASSERT_NE(set.End(), set.Insert(4).iterator);
    ASSERT_NE(set.End(), set.Insert(3).iterator);
    ASSERT_NE(set.End(), set.Insert(2).iterator);
    ASSERT_NE(set.End(), set.Insert(1).iterator);
    ASSERT_NE(set.End(), set.Insert(0).iterator);
    EXPECT_EQ(5, set.Size());

    int i = 0;
    for (const auto& val : set)
    {
        EXPECT_EQ(i, val) << "i = " << i;
        i++;
    }
    ASSERT_EQ(5, i);
}

/*
TEST(Set, Erase)
{
    const int size = 10;
    Set<int> set;

    for (int i = 0; i < size; ++i)
        ASSERT_NE(set.End(), set.Insert(i).iterator);

    {
        ASSERT_TRUE(set.Erase(0));
        EXPECT_EQ(set.End(), set.Find(0));
        EXPECT_FALSE(set.Erase(0));

        int i = 1;
        for (const auto& val : set)
        {
            EXPECT_EQ(i, val) << "i = " << i;
            i++;
        }
        ASSERT_EQ(size - 1, i);
    }
}
*/

TEST(Set, BigTest)
{
    // prepare some random data
    const int numValues = 10000;
    std::vector<int> values;
    values.reserve(numValues);

    for (int i = 0; i < numValues; ++i)
        values.push_back(i);

    Math::Random random(0);
    random.ShuffleContainer(values.begin(), values.end(), numValues);

    Set<int> set;

    for (int i = 0; i < numValues; ++i)
    {
        int value = values[i];
        ASSERT_NE(set.End(), set.Insert(value).iterator) << "i=" << i << ", val=" << value;
    }

    int i = 0;
    for (const auto& val : set)
    {
        EXPECT_EQ(i, val) << "i = " << i;
        i++;
    }
}