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
    EXPECT_EQ(1, array[0]);
    EXPECT_EQ(2, array[1]);
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

TEST(DynArray, CopyConstructor)
{
    using Type = CopyOnlyTestClass<int>;
    using ArrayType = DynArray<Type>;

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
        ArrayType array;
        ASSERT_NE(array.End(), array.PushBack(value1));
        ASSERT_NE(array.End(), array.PushBack(value2));
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // copy constructor
        const ArrayType set2(array);
        // array = []
        // set2 = [1, 2]

        ArrayType::ConstIterator iter;
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
        const ArrayType& constSet = array;
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
    using ArrayType = DynArray<Type>;

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
        ArrayType array;
        ASSERT_NE(array.End(), array.PushBack(value1));
        ASSERT_NE(array.End(), array.PushBack(value2));
        // array = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        ArrayType set2;
        ASSERT_NE(set2.End(), set2.PushBack(value3));
        ASSERT_NE(set2.End(), set2.PushBack(value4));
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
    using ArrayType = DynArray<Type>;

    ClassMethodCallCounters counters;

    {
        ArrayType array;

        // insert two initial elements
        ASSERT_NE(array.End(), array.PushBack(Type(&counters, 1)));
        ASSERT_NE(array.End(), array.PushBack(Type(&counters, 2)));
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
        ASSERT_NE(array.End(), array.PushBack(Type(&counters, 2)));
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
    using ArrayType = DynArray<Type>;

    ClassMethodCallCounters counters;
    {
        // initialize source DynArray
        ArrayType array;
        ASSERT_NE(array.End(), array.PushBack(Type(&counters, 1)));
        ASSERT_NE(array.End(), array.PushBack(Type(&counters, 2)));
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // call the move constructor
        ArrayType set2(std::move(array));
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
    using ArrayType = DynArray<Type>;

    ClassMethodCallCounters counters;
    {
        // initialize target array
        ArrayType array;
        ASSERT_NE(array.End(), array.PushBack(Type(&counters, 1)));
        ASSERT_NE(array.End(), array.PushBack(Type(&counters, 2)));
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // initialize source array
        ArrayType set2;
        ASSERT_NE(set2.End(), set2.PushBack(Type(&counters, 3)));
        ASSERT_NE(set2.End(), set2.PushBack(Type(&counters, 4)));
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
