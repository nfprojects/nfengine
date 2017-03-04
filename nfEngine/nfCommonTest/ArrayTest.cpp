/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for DynArray
 */

#include "PCH.hpp"
#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Math/Random.hpp"

#include "TestClasses.hpp"

using namespace NFE;
using namespace NFE::Common;

TEST(DynArray, Empty)
{
    DynArray<int> array;

    EXPECT_EQ(0, array.Size());
    EXPECT_TRUE(array.Empty());
    EXPECT_EQ(array.Begin(), array.End());
    EXPECT_EQ(array.End(), array.Find(0));
    EXPECT_FALSE(array.Erase(array.End()));
}

TEST(DynArray, PushBack)
{
    DynArray<int> array;

    const auto iter1 = array.PushBack(1);
    ASSERT_NE(array.End(), iter1);
    ASSERT_EQ(1, array.Size());

    const auto iter2 = array.PushBack(2);
    ASSERT_NE(array.End(), iter2);
    ASSERT_EQ(2, array.Size());

    ASSERT_NE(iter1, iter2);
}

TEST(DynArray, Clear)
{
    const uint32 numElements = 20;
    DynArray<int> array;

    for (uint32 i = 0; i < numElements; ++i)
    {
        array.PushBack(i);
    }

    ASSERT_EQ(numElements, array.Size());

    array.Clear();
    EXPECT_EQ(0, array.Size());
    EXPECT_TRUE(array.Empty());
}

/*
TEST(DynArray, CopyConstructor)
{
    using Type = CopyOnlyTestClass<int>;
    using SetType = DynArray<Type, Type::Comparator>;

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
        SetType array;
        ASSERT_NE(array.End(), array.Insert(value1).iterator);
        ASSERT_NE(array.End(), array.Insert(value2).iterator);
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // copy constructor
        const SetType set2(array);
        // array = []
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

        ASSERT_EQ(2, array.Size());
        ASSERT_EQ(2, set2.Size());

        // convert "array" to const to force use of ConstIterator
        const SetType& constSet = array;
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

TEST(DynArray, Assign)
{
    using Type = CopyOnlyTestClass<int>;
    using SetType = DynArray<Type, Type::Comparator>;

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
        SetType array;
        ASSERT_NE(array.End(), array.Insert(value1).iterator);
        ASSERT_NE(array.End(), array.Insert(value2).iterator);
        // array = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        SetType set2;
        ASSERT_NE(set2.End(), set2.Insert(value3).iterator);
        ASSERT_NE(set2.End(), set2.Insert(value4).iterator);
        // array = [1, 2]
        // set2 = [3, 4]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(4, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // assignment operator
        set2 = array;
        // array = [1, 2]
        // set2 = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(6, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(2, counters.destructor);

        ASSERT_EQ(2, array.Size());
        ASSERT_EQ(2, set2.Size());

        // convert "array" to const to force use of ConstIterator
        auto iter = array.Find(value1);
        ASSERT_NE(array.End(), iter);
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

TEST(DynArray, InsertViaMove)
{
    using Type = MoveOnlyTestClass<int>;
    using SetType = DynArray<Type, Type::Comparator>;

    ClassMethodCallCounters counters;

    {
        SetType array;

        // insert two initial elements
        ASSERT_NE(array.End(), array.Insert(Type(&counters, 1)).iterator);
        ASSERT_NE(array.End(), array.Insert(Type(&counters, 2)).iterator);
        ASSERT_EQ(2, array.Size());
        // array = [1, 2]

        // 2 elements created, 2 elements moved
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // insert existing element - the old one should be deleted
        ASSERT_NE(array.End(), array.Insert(Type(&counters, 2)).iterator);
        ASSERT_EQ(2, array.Size());
        // array = [1, 2]

        EXPECT_EQ(3, counters.constructor);
        EXPECT_EQ(3, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(1, counters.destructor);
    }

    EXPECT_EQ(3, counters.constructor);
    EXPECT_EQ(3, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(3, counters.destructor);
}

TEST(DynArray, MoveConstructor)
{
    using Type = MoveOnlyTestClass<int>;
    using SetType = DynArray<Type, Type::Comparator>;

    ClassMethodCallCounters counters;
    {
        // initialize source DynArray
        SetType array;
        ASSERT_NE(array.End(), array.Insert(Type(&counters, 1)).iterator);
        ASSERT_NE(array.End(), array.Insert(Type(&counters, 2)).iterator);
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // call the move constructor
        SetType set2(std::move(array));
        // array = []
        // set2 = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // "array" should be now empty
        EXPECT_EQ(0, array.Size());

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

TEST(DynArray, MoveAssignment)
{
    using Type = MoveOnlyTestClass<int>;
    using SetType = DynArray<Type, Type::Comparator>;

    ClassMethodCallCounters counters;
    {
        // initialize target array
        SetType array;
        ASSERT_NE(array.End(), array.Insert(Type(&counters, 1)).iterator);
        ASSERT_NE(array.End(), array.Insert(Type(&counters, 2)).iterator);
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // initialize source array
        SetType set2;
        ASSERT_NE(set2.End(), set2.Insert(Type(&counters, 3)).iterator);
        ASSERT_NE(set2.End(), set2.Insert(Type(&counters, 4)).iterator);
        // array = [1, 2]
        // set2 = [3, 4]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(4, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // move assign
        set2 = std::move(array);
        // array = []
        // set2 = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(4, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(2, counters.destructor);

        // "array" should be now empty
        EXPECT_EQ(0, array.Size());

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

TEST(DynArray, Iterator)
{
    DynArray<int> array;
    ASSERT_NE(array.End(), array.Insert(1).iterator);
    ASSERT_NE(array.End(), array.Insert(4).iterator);
    ASSERT_NE(array.End(), array.Insert(2).iterator);
    ASSERT_NE(array.End(), array.Insert(3).iterator);
    ASSERT_NE(array.End(), array.Insert(0).iterator);
    EXPECT_EQ(5, array.Size());
    ASSERT_TRUE(array.Verify());

    auto iter = array.Begin();
    EXPECT_EQ(0, *iter);

    ++iter;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(1, *iter);

    ++iter;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(2, *iter);

    ++iter;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(3, *iter);

    ++iter;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_NE(array.End(), iter);
    EXPECT_EQ(4, *iter);

    ++iter;
    ASSERT_NE(array.Begin(), iter);
    ASSERT_EQ(array.End(), iter);
}
*/

/*
TEST(DynArray, Find)
{
    const int size = 10;
    DynArray<int> array;

    for (int i = 0; i < size; ++i)
        ASSERT_NE(array.End(), array.Insert(i).iterator);

    for (int i = 0; i < size; ++i)
    {
        auto iter = array.Find(i);
        ASSERT_NE(array.End(), iter);
        EXPECT_EQ(i, *iter);
    }

    EXPECT_EQ(array.End(), array.Find(1234));
    EXPECT_EQ(array.End(), array.Find(-1234));
}

TEST(DynArray, RangeBasedFor)
{
    DynArray<int> array;
    ASSERT_NE(array.End(), array.Insert(4).iterator);
    ASSERT_NE(array.End(), array.Insert(3).iterator);
    ASSERT_NE(array.End(), array.Insert(2).iterator);
    ASSERT_NE(array.End(), array.Insert(1).iterator);
    ASSERT_NE(array.End(), array.Insert(0).iterator);
    EXPECT_EQ(5, array.Size());

    int i = 0;
    for (auto val : array)
    {
        EXPECT_EQ(i, val) << "i = " << i;
        i++;
    }
    ASSERT_EQ(5, i);
}
*/