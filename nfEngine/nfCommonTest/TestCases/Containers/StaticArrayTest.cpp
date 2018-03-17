/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for StaticArray
 */

#include "PCH.hpp"
#include "nfCommon/Containers/StaticArray.hpp"

#include "TestClasses.hpp"

using namespace NFE;
using namespace NFE::Common;

static const uint32 DefaultSize = 16;

TEST(StaticArray, Empty)
{
    StaticArray<int, DefaultSize> array;

    EXPECT_EQ(0u, array.Size());
    EXPECT_TRUE(array.Empty());
    EXPECT_EQ(array.Begin(), array.End());
    EXPECT_EQ(array.End(), array.GetView().Find(0));
}

TEST(StaticArray, Empty_Const)
{
    const StaticArray<int, DefaultSize> array;

    EXPECT_EQ(0u, array.Size());
    EXPECT_TRUE(array.Empty());
    EXPECT_EQ(array.Begin(), array.End());
    EXPECT_EQ(array.End(), array.GetView().Find(0));
}

TEST(StaticArray, PushBack)
{
    StaticArray<int, DefaultSize> array;

    const auto iter1 = array.PushBack(1);
    ASSERT_NE(array.End(), iter1);
    ASSERT_EQ(1u, array.Size());

    const auto iter2 = array.PushBack(2);
    ASSERT_NE(array.End(), iter2);
    ASSERT_EQ(2u, array.Size());

    ASSERT_NE(iter1, iter2);
    EXPECT_EQ(1, array[0]);
    EXPECT_EQ(2, array[1]);
}

TEST(StaticArray, InitializeFromArray)
{
    const int elements[] = { 10, 20, 30 };
    StaticArray<int, DefaultSize> array(elements, 3);

    ASSERT_EQ(3u, array.Size());
    EXPECT_EQ(10, array[0]);
    EXPECT_EQ(20, array[1]);
    EXPECT_EQ(30, array[2]);
}

TEST(StaticArray, Clear)
{
    StaticArray<int, DefaultSize> array;

    for (uint32 i = 0; i < DefaultSize; ++i)
    {
        array.PushBack(i);
    }

    ASSERT_EQ(DefaultSize, array.Size());

    array.Clear();
    EXPECT_EQ(0u, array.Size());
    EXPECT_TRUE(array.Empty());
}

TEST(StaticArray, Resize)
{
    struct Type
    {
        int a;
        Type() : a(1234) { }

        bool operator==(const Type& other) const
        {
            return a == other.a;
        }
    };

    StaticArray<Type, DefaultSize> array;
    ASSERT_TRUE(array.Resize(5));

    const Type defaultVal;
    EXPECT_EQ(5u, array.Size());
    EXPECT_EQ(defaultVal, array[0]);
    EXPECT_EQ(defaultVal, array[1]);
    EXPECT_EQ(defaultVal, array[2]);
    EXPECT_EQ(defaultVal, array[3]);
    EXPECT_EQ(defaultVal, array[4]);
}

TEST(StaticArray, InitializerList)
{
    const StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });

    EXPECT_EQ(5u, array.Size());
    EXPECT_FALSE(array.Empty());

    EXPECT_EQ(10, array[0]);
    EXPECT_EQ(20, array[1]);
    EXPECT_EQ(30, array[2]);
    EXPECT_EQ(40, array[3]);
    EXPECT_EQ(50, array[4]);
}

TEST(StaticArray, PopBack)
{
    int initialElements[] = { 10, 20, 30, 40 };
    StaticArray<int, DefaultSize> array(initialElements, 4);

    ASSERT_TRUE(array.PopBack());
    ASSERT_EQ(3u, array.Size());
    EXPECT_EQ(ArrayView<int>(initialElements, 3), array.GetView());

    ASSERT_TRUE(array.PopBack());
    ASSERT_EQ(2u, array.Size());
    EXPECT_EQ(ArrayView<int>(initialElements, 2), array.GetView());

    ASSERT_TRUE(array.PopBack());
    ASSERT_EQ(1u, array.Size());
    EXPECT_EQ(ArrayView<int>(initialElements, 1), array.GetView());

    ASSERT_TRUE(array.PopBack());
    ASSERT_EQ(0u, array.Size());

    ASSERT_FALSE(array.PopBack());
}

TEST(StaticArray, CopyConstructor)
{
    using Type = CopyOnlyTestClass<int>;
    using ArrayType = StaticArray<Type, DefaultSize>;

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
        {
            auto iter = array.PushBack(value1);
            ASSERT_NE(array.End(), iter);
            iter = array.PushBack(value2);
            ASSERT_NE(array.End(), iter);
        }
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // copy constructor
        const ArrayType array2(array);
        // array = []
        // array2 = [1, 2]

        const Type* ptr;
        const Type* ptr2;

        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(4, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        ASSERT_EQ(2u, array.Size());
        ASSERT_EQ(2u, array2.Size());

        // convert "array" to const to force use of ConstIterator
        const ArrayType& constArray = array;

        {
            auto iter = constArray.GetView().Find(value1);
            ASSERT_NE(constArray.End(), iter);
            ptr = &(*iter);
        }

        {
            auto iter = array2.GetView().Find(value1);
            ASSERT_NE(array2.End(), iter);
            ptr2 = &(*iter);
        }

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

TEST(StaticArray, Assign)
{
    using Type = CopyOnlyTestClass<int>;
    using ArrayType = StaticArray<Type, DefaultSize>;

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
        {
            auto iter = array.PushBack(value1);
            ASSERT_NE(array.End(), iter);
            iter = array.PushBack(value2);
            ASSERT_NE(array.End(), iter);
        }
        // array = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(2, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        ArrayType array2;
        {
            auto iter = array2.PushBack(value3);
            ASSERT_NE(array2.End(), iter);
            iter = array2.PushBack(value4);
            ASSERT_NE(array2.End(), iter);
        }
        // array = [1, 2]
        // array2 = [3, 4]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(4, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // assignment operator
        array2 = array;
        // array = [1, 2]
        // array2 = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(0, counters.moveConstructor);
        EXPECT_EQ(6, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(2, counters.destructor);

        ASSERT_EQ(2u, array.Size());
        ASSERT_EQ(2u, array2.Size());

        // convert "array" to const to force use of ConstIterator
        auto iter = array.GetView().Find(value1);
        ASSERT_NE(array.End(), iter);
        const Type* ptr = &(*iter);

        iter = array2.GetView().Find(value1);
        ASSERT_NE(array2.End(), iter);
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

TEST(StaticArray, MoveConstructor)
{
    using Type = MoveOnlyTestClass<int>;
    using ArrayType = StaticArray<Type, DefaultSize>;

    ClassMethodCallCounters counters;
    {
        // initialize source StaticArray
        ArrayType array;
        {
            auto iter = array.PushBack(Type(&counters, 1));
            ASSERT_NE(array.End(), iter);
            iter = array.PushBack(Type(&counters, 2));
            ASSERT_NE(array.End(), iter);
        }
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // call the move constructor
        ArrayType array2(std::move(array));
        // array = []
        // array2 = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(4, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // "array" should be now empty
        EXPECT_EQ(0u, array.Size());

        // check if elements were moved
        EXPECT_EQ(2u, array2.Size());
        EXPECT_NE(array2.End(), array2.GetView().Find(Type(nullptr, 1)));
        EXPECT_NE(array2.End(), array2.GetView().Find(Type(nullptr, 2)));
    }

    EXPECT_EQ(2, counters.constructor);
    EXPECT_EQ(4, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(0, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(2, counters.destructor);
}

TEST(StaticArray, MoveAssignment)
{
    using Type = MoveOnlyTestClass<int>;
    using ArrayType = StaticArray<Type, DefaultSize>;

    ClassMethodCallCounters counters;
    {
        // initialize target array
        ArrayType array;
        {
            auto iter = array.PushBack(Type(&counters, 1));
            ASSERT_NE(array.End(), iter);
            iter = array.PushBack(Type(&counters, 2));
            ASSERT_NE(array.End(), iter);
        }
        // array = [1, 2]
        EXPECT_EQ(2, counters.constructor);
        EXPECT_EQ(2, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // initialize source array
        ArrayType array2;
        {
            auto iter = array2.PushBack(Type(&counters, 3));
            ASSERT_NE(array2.End(), iter);
            iter = array2.PushBack(Type(&counters, 4));
            ASSERT_NE(array2.End(), iter);
        }
        // array = [1, 2]
        // array2 = [3, 4]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(4, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(0, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(0, counters.destructor);

        // move assign
        array2 = std::move(array);
        // array = []
        // array2 = [1, 2]
        EXPECT_EQ(4, counters.constructor);
        EXPECT_EQ(4, counters.moveConstructor);
        EXPECT_EQ(0, counters.copyConstructor);
        EXPECT_EQ(2, counters.moveAssignment);
        EXPECT_EQ(0, counters.assignment);
        EXPECT_EQ(2, counters.destructor);

        // "array" should be now empty
        EXPECT_EQ(0u, array.Size());

        // check if elements were moved
        EXPECT_EQ(2u, array2.Size());
        EXPECT_NE(array2.End(), array2.GetView().Find(Type(nullptr, 1)));
        EXPECT_NE(array2.End(), array2.GetView().Find(Type(nullptr, 2)));
    }

    EXPECT_EQ(4, counters.constructor);
    EXPECT_EQ(4, counters.moveConstructor);
    EXPECT_EQ(0, counters.copyConstructor);
    EXPECT_EQ(2, counters.moveAssignment);
    EXPECT_EQ(0, counters.assignment);
    EXPECT_EQ(4, counters.destructor);
}

TEST(StaticArray, PushBackArray)
{
    StaticArray<int, DefaultSize> array;
    array.PushBack(10);
    array.PushBack(20);
    array.PushBack(30);
    array.PushBack(40);

    int elements[] = { 100, 200, 300, 400, 500 };
    const ArrayView<int> toInsert(elements, 5);

    ASSERT_TRUE(array.PushBackArray(toInsert));
    ASSERT_EQ(9u, array.Size());

    int expectedElements[] = { 10, 20, 30, 40, 100, 200, 300, 400, 500 };
    const ArrayView<int> expected(expectedElements, 9);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, PushBackConstArray)
{
    StaticArray<int, DefaultSize> array;
    array.PushBack(10);
    array.PushBack(20);
    array.PushBack(30);
    array.PushBack(40);

    const int elements[] = { 100, 200, 300, 400, 500 };
    const ArrayView<const int> toInsert(elements, 5);

    ASSERT_TRUE(array.PushBackArray(toInsert));
    ASSERT_EQ(9u, array.Size());

    int expectedElements[] = { 10, 20, 30, 40, 100, 200, 300, 400, 500 };
    const ArrayView<int> expected(expectedElements, 9);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, InsertAt_Front)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertAt(0, 111);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 111, 10, 20, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 6);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, InsertAt_Back)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertAt(5, 111);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 20, 30, 40, 50, 111 };
    const ArrayView<int> expected(expectedElements, 6);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, InsertAt_Middle)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertAt(2, 111);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 20, 111, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 6);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, InsertArrayAt_Empty)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });
    const ArrayView<int> toInsert;

    auto iter = array.InsertArrayAt(0, toInsert);
    ASSERT_EQ(array.End(), iter);

    iter = array.InsertArrayAt(2, toInsert);
    ASSERT_EQ(array.End(), iter);

    iter = array.InsertArrayAt(5, toInsert);
    ASSERT_EQ(array.End(), iter);
}

TEST(StaticArray, InsertArrayAt_Front)
{
    int elementsToInsert[] = { 111, 222, 333 };
    const ArrayView<int> toInsert(elementsToInsert, 3);

    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertArrayAt(0, toInsert);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 111, 222, 333, 10, 20, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 8);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, InsertAt_Multiple)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertAt(1, 111, 3);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 111, 111, 111, 20, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 8);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, InsertArrayAt_Back)
{
    int elementsToInsert[] = { 111, 222, 333 };
    const ArrayView<int> toInsert(elementsToInsert, 3);

    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertArrayAt(5, toInsert);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 20, 30, 40, 50, 111, 222, 333 };
    const ArrayView<int> expected(expectedElements, 8);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, InsertArrayAt_Middle)
{
    int elementsToInsert[] = { 111, 222, 333 };
    const ArrayView<int> toInsert(elementsToInsert, 3);

    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });
    auto iter = array.InsertArrayAt(2, toInsert);
    ASSERT_NE(array.End(), iter);

    int expectedElements[] = { 10, 20, 111, 222, 333, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 8);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, Erase_Front)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin()));

    int expectedElements[] = { 20, 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 4);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, Erase_Middle)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin() + 2));

    int expectedElements[] = { 10, 20, 40, 50 };
    const ArrayView<int> expected(expectedElements, 4);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, Erase_Back)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.End() - 1));

    int expectedElements[] = { 10, 20, 30, 40 };
    const ArrayView<int> expected(expectedElements, 4);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, Erase_Range_Front)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin(), array.Begin() + 2));

    int expectedElements[] = { 30, 40, 50 };
    const ArrayView<int> expected(expectedElements, 3);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, Erase_Range_Middle)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin() + 1, array.Begin() + 3));

    int expectedElements[] = { 10, 40, 50 };
    const ArrayView<int> expected(expectedElements, 3);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, Erase_Range_End)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin() + 3, array.End()));

    int expectedElements[] = { 10, 20, 30 };
    const ArrayView<int> expected(expectedElements, 3);
    EXPECT_EQ(expected, array.GetView());
}

TEST(StaticArray, Erase_Range_Everything)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });

    EXPECT_TRUE(array.Erase(array.Begin(), array.End()));
    ASSERT_TRUE(array.Empty());
}

TEST(StaticArray, Erase_Range_Empty)
{
    StaticArray<int, DefaultSize> array({ 10, 20, 30, 40, 50 });

    EXPECT_FALSE(array.Erase(array.Begin(), array.Begin()));
    ASSERT_EQ(5u, array.Size());

    EXPECT_FALSE(array.Erase(array.Begin() + 1, array.Begin()));
    ASSERT_EQ(5u, array.Size());

    EXPECT_FALSE(array.Erase(array.Begin() + 1, array.Begin() + 1));
    ASSERT_EQ(5u, array.Size());

    EXPECT_FALSE(array.Erase(array.End(), array.End()));
    ASSERT_EQ(5u, array.Size());
}