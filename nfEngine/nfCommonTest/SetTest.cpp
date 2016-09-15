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

TEST(Set, Clear)
{
    Set<int> set;

    ASSERT_NE(set.End(), set.Insert(1).iterator);
    ASSERT_NE(set.End(), set.Insert(2).iterator);
    EXPECT_EQ(2, set.Size());
    EXPECT_FALSE(set.Empty());

    set.Clear();
    EXPECT_EQ(0, set.Size());
    EXPECT_TRUE(set.Empty());
}

TEST(Set, CopyConstructor)
{
    Set<int> set;
    ASSERT_NE(set.End(), set.Insert(1).iterator);
    ASSERT_NE(set.End(), set.Insert(2).iterator);

    const Set<int> set2(set);
    Set<int>::ConstIterator iter;
    const int* ptr;
    const int* ptr2;

    ASSERT_EQ(2, set.Size());
    ASSERT_EQ(2, set2.Size());

    const Set<int>* constSet = &set;
    iter = constSet->Find(1);
    ASSERT_NE(constSet->End(), iter);
    ptr = &(*iter);
    iter = set2.Find(1);
    ASSERT_NE(set2.End(), iter);
    ptr2 = &(*iter);
    EXPECT_NE(ptr, ptr2);
}

TEST(Set, Assign)
{
    Set<int> set, set2;
    ASSERT_NE(set.End(), set.Insert(1).iterator);
    ASSERT_NE(set.End(), set.Insert(2).iterator);

    Set<int>::ConstIterator iter;
    const int* ptr;
    const int* ptr2;

    ASSERT_EQ(0, set2.Size());
    set2 = set;
    ASSERT_EQ(2, set.Size());
    ASSERT_EQ(2, set2.Size());

    const Set<int>* constSet = &set;
    iter = constSet->Find(1);
    ASSERT_NE(set.End(), iter);
    ptr = &(*iter);

    const Set<int>* constSet2 = &set2;
    iter = constSet2->Find(1);
    ASSERT_NE(set2.End(), iter);
    ptr2 = &(*iter);
    EXPECT_NE(ptr, ptr2);
}

TEST(Set, MoveConstructor)
{
    Set<int> set;
    ASSERT_NE(set.End(), set.Insert(1).iterator);
    ASSERT_NE(set.End(), set.Insert(2).iterator);

    Set<int> set2(std::move(set));

    ASSERT_EQ(0, set.Size());
    ASSERT_EQ(2, set2.Size());

    const auto iter = set2.Find(1);
    ASSERT_NE(set2.End(), iter);
}

TEST(Set, MoveAssignment)
{
    Set<int> set, set2;
    ASSERT_NE(set.End(), set.Insert(1).iterator);
    ASSERT_NE(set.End(), set.Insert(2).iterator);

    ASSERT_EQ(0, set2.Size());
    set2 = std::move(set);
    ASSERT_EQ(0, set.Size());
    ASSERT_EQ(2, set2.Size());

    const auto iter = set2.Find(1);
    ASSERT_NE(set2.End(), iter);
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
    ASSERT_TRUE(set.Verify());

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
    for (auto val : set)
    {
        EXPECT_EQ(i, val) << "i = " << i;
        i++;
    }
    ASSERT_EQ(5, i);
}

TEST(Set, EraseSingleElementTree)
{
    Set<int> set;
    ASSERT_NE(set.End(), set.Insert(0).iterator);

    ASSERT_TRUE(set.Erase(0));
    ASSERT_EQ(0, set.Size());
    ASSERT_TRUE(set.Verify());

    EXPECT_EQ(set.End(), set.Find(0));
    EXPECT_FALSE(set.Erase(0));
}

TEST(Set, EraseBasic)
{
    const int size = 10;
    Set<int> set;

    for (int i = 0; i < size; ++i)
        ASSERT_NE(set.End(), set.Insert(i).iterator);

    ASSERT_TRUE(set.Erase(0));
    EXPECT_EQ(set.End(), set.Find(0));
    EXPECT_FALSE(set.Erase(0));
    ASSERT_EQ(size - 1, set.Size());
    ASSERT_TRUE(set.Verify());
}

TEST(Set, BigTest)
{
    // prepare some random data
    const int numValues = 10000;
    std::vector<int> values;
    values.reserve(numValues);

    for (int i = 0; i < numValues; ++i)
        values.push_back(i);

    Set<int> set;

    // insert values in random order
    Math::Random random(0);
    random.ShuffleContainer(values.begin(), values.end(), numValues);
    for (int i = 0; i < numValues; ++i)
    {
        int value = values[i];
        ASSERT_NE(set.End(), set.Insert(value).iterator) << "i=" << i << ", val=" << value;
    }

    ASSERT_TRUE(set.Verify());

    int j = 0;
    for (const auto& val : set)
    {
        EXPECT_EQ(j, val) << "j = " << j;
        j++;
    }

    // remove random values
    const int valuesToRemove = numValues / 10;
    random.ShuffleContainer(values.begin(), values.end(), numValues);
    for (int i = 0; i < valuesToRemove; ++i)
    {
        int value = values[i];
        ASSERT_TRUE(set.Erase(value));
    }

    ASSERT_TRUE(set.Verify());

    for (int i = 0; i < valuesToRemove; ++i)
    {
        int value = values[i];
        ASSERT_EQ(set.End(), set.Find(value));
    }

    for (int i = valuesToRemove; i < numValues; ++i)
    {
        int value = values[i];
        ASSERT_NE(set.End(), set.Find(value));
    }
}

TEST(Set, Permutations)
{
    Math::Random random(1);
    std::vector<int> values;
    
    // build trees containing from 1 to 6 elements
    for (int numValues = 1; numValues <= 6; ++numValues)
    {
        values.clear();
        for (int i = 0; i < numValues; ++i)
            values.push_back(i);

        // generate all possible permutations
        int permutation = 0;
        do
        {
            std::string trace = "numValues = " + std::to_string(numValues) + ", permutation = " + std::to_string(permutation);
            SCOPED_TRACE(trace);

            // initialize the tree
            Set<int> set;
            for (int i = 0; i < numValues; ++i)
                ASSERT_NE(set.End(), set.Insert(i).iterator);

            // "Find" test
            for (int i = 0; i < numValues; ++i)
            {
                auto iter = set.Find(i);
                ASSERT_NE(set.End(), iter);
                EXPECT_EQ(i, *iter);
            }

            ASSERT_TRUE(set.Verify());

            // try to remove not-existing value
            ASSERT_FALSE(set.Erase(numValues));

            int toRemove = static_cast<uint32>(random.GetInt()) % numValues;
            ASSERT_TRUE(set.Erase(toRemove)) << "toRemove = " << toRemove;

            ASSERT_TRUE(set.Verify());

            // "Find" test again
            for (int i = 0; i < numValues; ++i)
            {
                auto iter = set.Find(i);

                if (toRemove == i)
                {
                    ASSERT_EQ(set.End(), iter);
                }
                else
                {
                    ASSERT_NE(set.End(), iter);
                    EXPECT_EQ(i, *iter);
                }
            }

            permutation++;
        } while (std::next_permutation(values.begin(), values.end()));
    }
}