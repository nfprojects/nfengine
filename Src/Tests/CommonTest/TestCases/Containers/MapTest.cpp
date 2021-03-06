/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Map container
 */

#include "PCH.hpp"
#include "Engine/Common/Containers/Map.hpp"
#include "Engine/Common/Containers/String.hpp"


using namespace NFE;
using namespace NFE::Common;

TEST(Map, Empty)
{
    Map<int, int> map;

    EXPECT_EQ(0u, map.Size());
    EXPECT_TRUE(map.Empty());
    EXPECT_EQ(map.Begin(), map.End());
    EXPECT_EQ(map.End(), map.Find(0));
    EXPECT_FALSE(map.Erase(0));
    EXPECT_FALSE(map.Erase(map.End()));
}

TEST(Map, Insert)
{
    Map<int, int> map;

    {
        const auto result = map.Insert(1, 2);
        ASSERT_NE(map.End(), result.iterator);
        ASSERT_FALSE(result.replaced);
        EXPECT_EQ(2, map[1]);
    }

    {
        const auto result = map.Insert(1, 3);
        ASSERT_EQ(map.End(), result.iterator);
        ASSERT_FALSE(result.replaced);
        EXPECT_EQ(2, map[1]);
    }

    EXPECT_NE(map.End(), map.Find(1));
    EXPECT_EQ(map.End(), map.Find(2));
}

TEST(Map, InsertOrReplace)
{
    Map<int, int> map;

    {
        const auto result = map.InsertOrReplace(1, 2);
        ASSERT_NE(map.End(), result.iterator);
        ASSERT_FALSE(result.replaced);

        // map = { 1 -> 2 }
        EXPECT_EQ(2, map[1]);
    }

    {
        const auto result = map.InsertOrReplace(1, 3);
        ASSERT_NE(map.End(), result.iterator);
        ASSERT_TRUE(result.replaced);

        // map = { 1 -> 3 }
        EXPECT_EQ(3, map[1]);
    }

    EXPECT_NE(map.End(), map.Find(1));
    EXPECT_EQ(map.End(), map.Find(2));
}

TEST(Map, Iterate)
{
    Map<uint32, float> map;

    const uint32 numElements = 10u;
    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_NE(map.End(), map.Insert(i, static_cast<float>(i)).iterator) << "i = " << i;
    }

    uint32 i = 0;
    for (const auto& pair : map)
    {
        EXPECT_EQ(i, pair.first) << "i = " << i;
        EXPECT_EQ(static_cast<float>(i), pair.second) << "i = " << i;
        ++i;
    }
}

TEST(Map, EraseByKey)
{
    Map<uint32, float> map;

    const uint32 numElements = 10u;
    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_NE(map.End(), map.Insert(i, static_cast<float>(i)).iterator) << "i = " << i;
    }

    ASSERT_EQ(numElements, map.Size());
    EXPECT_TRUE(map.Erase(5));
    ASSERT_EQ(numElements - 1, map.Size());

    EXPECT_EQ(map.End(), map.Find(5));
}

TEST(Map, EraseByIterator)
{
    Map<uint32, float> map;

    const uint32 numElements = 10u;
    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_NE(map.End(), map.Insert(i, static_cast<float>(i)).iterator) << "i = " << i;
    }

    ASSERT_EQ(numElements, map.Size());
    {
        const Map<uint32, float>::Iterator iter = map.Find(5);
        ASSERT_NE(map.End(), iter);
        EXPECT_TRUE(map.Erase(iter));
    }
    ASSERT_EQ(numElements - 1, map.Size());

    EXPECT_EQ(map.End(), map.Find(5));
}

TEST(Map, EraseByConstIterator)
{
    Map<uint32, float> map;

    const uint32 numElements = 10u;
    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_NE(map.End(), map.Insert(i, static_cast<float>(i)).iterator) << "i = " << i;
    }

    ASSERT_EQ(numElements, map.Size());
    {
        const Map<uint32, float>::ConstIterator iter = map.Find(5);
        ASSERT_NE(map.End(), iter);
        EXPECT_TRUE(map.Erase(iter));
    }
    ASSERT_EQ(numElements - 1, map.Size());

    EXPECT_EQ(map.End(), map.Find(5));
}

TEST(Map, StringToString)
{
    Map<String, String> map;

    ASSERT_NE(map.End(), map.Insert(String("a"), String("aaa")).iterator);
    ASSERT_NE(map.End(), map.Insert(String("b"), String("bbb")).iterator);

    ASSERT_NE(map.End(), map.Find(String("a")));
    ASSERT_NE(map.End(), map.Find(String("b")));
    ASSERT_EQ(map.End(), map.Find(String("c")));

    ASSERT_EQ(String("aaa"), map[String("a")]);
    ASSERT_EQ(String("bbb"), map[String("b")]);

    ASSERT_TRUE(map.Erase(String("a")));
    ASSERT_TRUE(map.Erase(String("b")));
    ASSERT_FALSE(map.Erase(String("c")));
}