/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Set
 */

#include "PCH.hpp"
#include "nfCommon/Containers/Set.hpp"
#include "nfCommon/Containers/String.hpp"
#include "nfCommon/Math/Random.hpp"

#include "TestClasses.hpp"

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

    ASSERT_EQ(set.End(), set.Insert(2).iterator);
    // set = [1,2]
    EXPECT_EQ(2, set.Size());
    EXPECT_FALSE(set.Empty());
}

TEST(Set, SimpleInsertOrReplace)
{
    Set<int> set;

    {
        const auto result = set.InsertOrReplace(1);
        ASSERT_NE(set.End(), result.iterator);
        ASSERT_FALSE(result.replaced);
        // set = [1]
        EXPECT_EQ(1, set.Size());
        EXPECT_FALSE(set.Empty());
    }

    {
        const auto result = set.InsertOrReplace(2);
        ASSERT_NE(set.End(), result.iterator);
        ASSERT_FALSE(result.replaced);
        // set = [1,2]
        EXPECT_EQ(2, set.Size());
        EXPECT_FALSE(set.Empty());
    }

    {
        const auto result = set.InsertOrReplace(2);
        ASSERT_NE(set.End(), result.iterator);
        ASSERT_TRUE(result.replaced);
        // set = [1,2]
        EXPECT_EQ(2, set.Size());
        EXPECT_FALSE(set.Empty());
    }
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
    using Type = CopyOnlyTestClass<int>;
    using SetType = Set<Type, Type::Comparator>;

    ClassMethodCallCounters counters;
    const Type value1(&counters, 1);
    const Type value2(&counters, 2);

    EXPECT_EQ(2, counters.constructor);
    EXPECT_EQ(0, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(0, counters.destructor);

    {
        SetType set;
        ASSERT_NE(set.End(), set.Insert(value1).iterator);
        ASSERT_NE(set.End(), set.Insert(value2).iterator);
        // set = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // copy constructor
        const SetType set2(set);
        // set = []
        // set2 = [1, 2]

        SetType::ConstIterator iter;
        const Type* ptr;
        const Type* ptr2;

        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(4, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        ASSERT_EQ(2, set.Size());
        ASSERT_EQ(2, set2.Size());

        // convert "set" to const to force use of ConstIterator
        const SetType& constSet = set;
        iter = constSet.Find(value1);
        ASSERT_NE(constSet.End(), iter);
        ptr = &(*iter);

        iter = set2.Find(value1);
        ASSERT_NE(set2.End(), iter);
        ptr2 = &(*iter);

        // compare pointer to the same value in both sets - they should be different
        EXPECT_NE(ptr, ptr2);
    }

    EXPECT_EQ(2, counters.constructor);
    EXPECT_EQ(0, counters.moveConstructor);
    EXPECT_EQ(4, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(4, counters.destructor);
}

TEST(Set, Assign)
{
    using Type = CopyOnlyTestClass<int>;
    using SetType = Set<Type, Type::Comparator>;

    ClassMethodCallCounters counters;
    const Type value1(&counters, 1);
    const Type value2(&counters, 2);
    const Type value3(&counters, 3);
    const Type value4(&counters, 4);

    EXPECT_EQ(4, counters.constructor);
    EXPECT_EQ(0, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(0, counters.destructor);

    {
        SetType set;
        ASSERT_NE(set.End(), set.Insert(value1).iterator);
        ASSERT_NE(set.End(), set.Insert(value2).iterator);
        // set = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        SetType set2;
        ASSERT_NE(set2.End(), set2.Insert(value3).iterator);
        ASSERT_NE(set2.End(), set2.Insert(value4).iterator);
        // set = [1, 2]
        // set2 = [3, 4]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(4, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // assignment operator
        set2 = set;
        // set = [1, 2]
        // set2 = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(6, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(2, counters.destructor);

        ASSERT_EQ(2, set.Size());
        ASSERT_EQ(2, set2.Size());

        // convert "set" to const to force use of ConstIterator
        auto iter = set.Find(value1);
        ASSERT_NE(set.End(), iter);
        const Type* ptr = &(*iter);

        iter = set2.Find(value1);
        ASSERT_NE(set2.End(), iter);
        const Type* ptr2 = &(*iter);

        // compare pointer to the same value in both sets - they should be different
        EXPECT_NE(ptr, ptr2);
    }

    EXPECT_EQ(4, counters.constructor);
    EXPECT_EQ(0, counters.moveConstructor);
    EXPECT_EQ(6, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(6, counters.destructor);
}

TEST(Set, InsertViaMove)
{
    using Type = MoveOnlyTestClass<int>;
    using SetType = Set<Type, Type::Comparator>;

    ClassMethodCallCounters counters;

    {
        SetType set;

        // insert two initial elements
        ASSERT_NE(set.End(), set.Insert(Type(&counters, 1)).iterator);
        ASSERT_NE(set.End(), set.Insert(Type(&counters, 2)).iterator);
        ASSERT_EQ(2, set.Size());
        // set = [1, 2]

        // 2 elements created, 2 elements moved
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // replace existing element - the old one should be deleted
        {
            const auto result = set.InsertOrReplace(Type(&counters, 2));
            ASSERT_NE(set.End(), result.iterator);
            ASSERT_TRUE(result.replaced);
            ASSERT_EQ(2, set.Size());
            // set = [1, 2]

            EXPECT_EQ(3, counters.constructor);
            EXPECT_EQ(3, counters.moveConstructor);
            EXPECT_EQ(0, counters.copyConstructor);
            EXPECT_EQ(0, counters.moveAssignment);
            EXPECT_EQ(0, counters.assignment);
            EXPECT_EQ(1, counters.destructor);
        }
    }

    EXPECT_EQ(3, counters.constructor);
    EXPECT_EQ(3, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(3, counters.destructor);
}

TEST(Set, MoveConstructor)
{
    using Type = MoveOnlyTestClass<int>;
    using SetType = Set<Type, Type::Comparator>;

    ClassMethodCallCounters counters;
    {
        // initialize source Set
        SetType set;
        ASSERT_NE(set.End(), set.Insert(Type(&counters, 1)).iterator);
        ASSERT_NE(set.End(), set.Insert(Type(&counters, 2)).iterator);
        // set = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // call the move constructor
        SetType set2(std::move(set));
        // set = []
        // set2 = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // "set" should be now empty
        EXPECT_EQ(0, set.Size());

        // check if elements were moved
        EXPECT_EQ(2, set2.Size());
        EXPECT_NE(set2.End(), set2.Find(Type(nullptr, 1)));
        EXPECT_NE(set2.End(), set2.Find(Type(nullptr, 2)));
    }

    EXPECT_EQ(2, counters.constructor);
    EXPECT_EQ(2, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(2, counters.destructor);
}

TEST(Set, MoveAssignment)
{
    using Type = MoveOnlyTestClass<int>;
    using SetType = Set<Type, Type::Comparator>;

    ClassMethodCallCounters counters;
    {
        // initialize target set
        SetType set;
        ASSERT_NE(set.End(), set.Insert(Type(&counters, 1)).iterator);
        ASSERT_NE(set.End(), set.Insert(Type(&counters, 2)).iterator);
        // set = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // initialize source set
        SetType set2;
        ASSERT_NE(set2.End(), set2.Insert(Type(&counters, 3)).iterator);
        ASSERT_NE(set2.End(), set2.Insert(Type(&counters, 4)).iterator);
        // set = [1, 2]
        // set2 = [3, 4]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(4, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // move assign
        set2 = std::move(set);
        // set = []
        // set2 = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(4, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(2, counters.destructor);

        // "set" should be now empty
        EXPECT_EQ(0, set.Size());

        // check if elements were moved
        EXPECT_EQ(2, set2.Size());
        EXPECT_NE(set2.End(), set2.Find(Type(nullptr, 1)));
        EXPECT_NE(set2.End(), set2.Find(Type(nullptr, 2)));
    }

    EXPECT_EQ(4, counters.constructor);
    EXPECT_EQ(4, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(4, counters.destructor);
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
    const int numValues = 1000;
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

//////////////////////////////////////////////////////////////////////////

TEST(Set, SetOfStrings)
{
    Set<String> set;

    ASSERT_NE(set.End(), set.Insert("aaa").iterator);
    ASSERT_NE(set.End(), set.Insert("bbb").iterator);

    ASSERT_NE(set.End(), set.Find("aaa"));
    ASSERT_NE(set.End(), set.Find("bbb"));
    ASSERT_EQ(set.End(), set.Find("ccc"));

    ASSERT_TRUE(set.Erase("aaa"));
    ASSERT_TRUE(set.Erase("bbb"));
    ASSERT_FALSE(set.Erase("ccc"));
}