#include "PCH.hpp"
#include "nfCommon/PackedArray.hpp"
#include "nfCommon/Math/Math.hpp"
#include "nfCommon/Math/Random.hpp"

#include "TestClasses.hpp"


using namespace NFE::Common;
using namespace NFE::Math;

namespace {

const int MAX_OBJECTS = 1000000;
const int TEST_VALUE = 123456;

} // namespace

TEST(PackedArray, AddUnsignedChar)
{
    PackedArray<int, unsigned char> array;

    const int maxObj = 0xFF;
    ASSERT_EQ(maxObj, array.MaxSize());

    for (int i = 0; i < maxObj; ++i)
    {
        unsigned char idx = array.Add(i);
        ASSERT_NE(array.InvalidIndex, idx) << "i=" << i;
    }
    ASSERT_EQ(maxObj, array.Size());

    unsigned char idx = array.Add(TEST_VALUE);
    ASSERT_EQ(array.InvalidIndex, idx);
    idx = array.Add(TEST_VALUE);
    ASSERT_EQ(array.InvalidIndex, idx);
}

TEST(PackedArray, AddUnsignedShort)
{
    PackedArray<int, unsigned short> array;

    const int maxObj = 0xFFFF;
    ASSERT_EQ(array.MaxSize(), maxObj);

    for (int i = 0; i < maxObj; ++i)
    {
        unsigned short idx = array.Add(i);
        ASSERT_NE(array.InvalidIndex, idx) << "i=" << i;
    }
    ASSERT_EQ(maxObj, array.Size());

    unsigned short idx = array.Add(TEST_VALUE);
    ASSERT_EQ(array.InvalidIndex, idx);
    idx = array.Add(TEST_VALUE);
    ASSERT_EQ(array.InvalidIndex, idx);
}

TEST(PackedArray, AddAndRemoveAscending)
{
    PackedArray<int, unsigned int> array;

    const unsigned int maxSize = 0xFFFFFFFF;
    ASSERT_EQ(maxSize, array.MaxSize());

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        unsigned int idx = array.Add(1000 * i);
        ASSERT_NE(array.InvalidIndex, idx) << "i=" << i;
    }
    ASSERT_EQ(MAX_OBJECTS, array.Size());

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        unsigned int idx = static_cast<unsigned int>(i);
        ASSERT_TRUE(array.Has(idx)) << "i=" << i;
        array.Remove(idx);
    }
    ASSERT_EQ(0, array.Size());

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        unsigned int idx = static_cast<unsigned int>(i);
        ASSERT_FALSE(array.Has(idx)) << "i=" << i;
    }
}

TEST(PackedArray, AddAndRemoveDescending)
{
    PackedArray<int, unsigned int> array;

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        unsigned int idx = array.Add(1000 * i);
        ASSERT_NE(array.InvalidIndex, idx) << "i=" << i;
    }
    ASSERT_EQ(MAX_OBJECTS, array.Size());

    for (int i = MAX_OBJECTS - 1; i >= 0; --i)
    {
        unsigned int idx = static_cast<unsigned int>(i);
        ASSERT_TRUE(array.Has(idx)) << "i=" << i;
        array.Remove(idx);
    }
    ASSERT_EQ(0, array.Size());

    for (int i = 0; i < MAX_OBJECTS; ++i)
    {
        unsigned int idx = static_cast<unsigned int>(i);
        ASSERT_FALSE(array.Has(idx)) << "i=" << i;
    }
}

TEST(PackedArray, AddAndRemoveRandom)
{
    PackedArray<int, unsigned int> array;
    std::map<unsigned int, int> map;

    const int maxObj = 64 * 1024;
    const int batchSize = 8 * 1024;
    const int iter = 10;

    Random random;

    // add initial objects
    for (int i = 0; i < maxObj; ++i)
    {
        int value = random.GetInt();
        unsigned int idx = array.Add(value);
        ASSERT_NE(array.InvalidIndex, idx) << "i=" << i;
        map[idx] = value;
    }
    ASSERT_EQ(maxObj, array.Size());

    for (int i = 0; i < iter; ++i)
    {
        for (int j = 0; j < batchSize; ++j)
        {
            unsigned int idx;
            do
            {
                idx = random.GetInt() % maxObj;
            } while (map.count(idx) == 0);

            ASSERT_TRUE(array.Has(idx)) << "i=" << i << ", j=" << j;
            array.Remove(idx);
            map.erase(idx);
        }
        ASSERT_EQ(maxObj - batchSize, array.Size()) << "i=" << i;


        // add new objects
        for (int j = 0; j < batchSize; ++j)
        {
            int value = random.GetInt();
            unsigned int idx = array.Add(value);
            ASSERT_NE(array.InvalidIndex, idx) << "i=" << i;
            ASSERT_EQ(0, map.count(idx)) << "i=" << i; // check if we get not duplicated index
            map[idx] = value;
        }
        ASSERT_EQ(maxObj, array.Size()) << "i=" << i;


        // verify
        for (const auto& it : map)
        {
            unsigned int idx = it.first;
            int value = it.second;
            ASSERT_EQ(value, array[idx]) << "i=" << i;
        }
    }
}

TEST(PackedArray, ComplexTypeCopy)
{
    using TestClass = CopyOnlyTestClass<int>;
    ClassMethodCallCounters counter;

    {
        PackedArray<TestClass> array;

        {
            TestClass obj(&counter);
            array.Add(obj);
        }

        EXPECT_EQ(1, counter.constructor);
        EXPECT_EQ(1, counter.copyConstructor);
        EXPECT_EQ(0, counter.assignment);
        EXPECT_EQ(1, counter.destructor); // original only
    }

    EXPECT_EQ(1, counter.constructor);
    EXPECT_EQ(1, counter.copyConstructor);
    EXPECT_EQ(0, counter.assignment);
    EXPECT_EQ(2, counter.destructor); // one for original, second for the copy
}

TEST(PackedArray, ComplexTypeMove)
{
    using TestClass = MoveOnlyTestClass<int>;
    ClassMethodCallCounters counter;

    {
        PackedArray<TestClass> array;

        {
            TestClass obj(&counter);
            array.Add(std::move(obj));
        }

        EXPECT_EQ(1, counter.constructor);
        EXPECT_EQ(1, counter.moveConstructor);
        EXPECT_EQ(0, counter.moveAssignment);
        EXPECT_EQ(0, counter.destructor);
    }

    EXPECT_EQ(1, counter.constructor);
    EXPECT_EQ(1, counter.moveConstructor);
    EXPECT_EQ(0, counter.moveAssignment);
    EXPECT_EQ(1, counter.destructor);
}