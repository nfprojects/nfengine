/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Set (when used with other containers)
 */

#include "PCH.hpp"
#include "Engine/Common/Containers/Set.hpp"
#include "Engine/Common/Containers/String.hpp"
#include "Engine/Common/Containers/SharedPtr.hpp"
#include "Engine/Common/Containers/DynArray.hpp"


using namespace NFE;
using namespace NFE::Common;

TEST(Set, SetOfStrings)
{
    Set<String> set;

    ASSERT_NE(set.End(), set.Insert(String("aaa")).iterator);
    ASSERT_NE(set.End(), set.Insert(String("bbb")).iterator);

    ASSERT_NE(set.End(), set.Find(String("aaa")));
    ASSERT_NE(set.End(), set.Find(String("bbb")));
    ASSERT_EQ(set.End(), set.Find(String("ccc")));

    ASSERT_TRUE(set.Erase(String("aaa")));
    ASSERT_TRUE(set.Erase(String("bbb")));
    ASSERT_FALSE(set.Erase(String("ccc")));
}

TEST(Set, SetOfDynArrays)
{
    using KeyType = DynArray<int>;
    Set<KeyType> set;

    KeyType array1, array2;
    array1.PushBack(1);
    array2.PushBack(2);
    KeyType arrayCopy1 = array1;
    KeyType arrayCopy2 = array2;

    ASSERT_NE(set.End(), set.Insert(array1).iterator);
    ASSERT_NE(set.End(), set.Insert(array2).iterator);

    ASSERT_NE(set.End(), set.Find(arrayCopy1));
    ASSERT_NE(set.End(), set.Find(arrayCopy2));
    ASSERT_EQ(set.End(), set.Find(KeyType()));

    ASSERT_TRUE(set.Erase(arrayCopy1));
    ASSERT_TRUE(set.Erase(arrayCopy2));
    ASSERT_FALSE(set.Erase(KeyType()));
}

TEST(Set, SetOfSharedPtrs)
{
    using KeyType = SharedPtr<int>;
    Set<KeyType> set;

    KeyType ptr1 = MakeSharedPtr<int>(1);
    KeyType ptr2 = MakeSharedPtr<int>(2);
    KeyType copyPtr1 = ptr1;
    KeyType copyPtr2 = ptr2;

    ASSERT_NE(set.End(), set.Insert(ptr1).iterator);
    ASSERT_NE(set.End(), set.Insert(ptr2).iterator);

    ASSERT_NE(set.End(), set.Find(copyPtr1));
    ASSERT_NE(set.End(), set.Find(copyPtr2));
    ASSERT_EQ(set.End(), set.Find(nullptr));

    ASSERT_TRUE(set.Erase(copyPtr1));
    ASSERT_TRUE(set.Erase(copyPtr2));
    ASSERT_FALSE(set.Erase(nullptr));
}

TEST(Set, SetOfUniquePtrs)
{
    using KeyType = UniquePtr<int>;
    Set<KeyType> set;

    KeyType ptr1 = MakeUniquePtr<int>(1);
    KeyType ptr2 = MakeUniquePtr<int>(2);
    KeyType ptr3 = MakeUniquePtr<int>(3);

    int* rawPtr1 = ptr1.Get();
    int* rawPtr2 = ptr2.Get();
    int* rawPtr3 = ptr3.Get();

    ASSERT_NE(set.End(), set.Insert(std::move(ptr1)).iterator);
    ASSERT_NE(set.End(), set.Insert(std::move(ptr2)).iterator);
    ASSERT_NE(set.End(), set.Insert(std::move(ptr3)).iterator);

    auto iter1 = std::find_if(set.begin(), set.end(), [&](const KeyType& key) { return key.Get() == rawPtr1; });
    auto iter2 = std::find_if(set.begin(), set.end(), [&](const KeyType& key) { return key.Get() == rawPtr2; });
    auto iter3 = std::find_if(set.begin(), set.end(), [&](const KeyType& key) { return key.Get() == rawPtr3; });

    ASSERT_EQ(3u, set.Size());
    ASSERT_NE(set.End(), iter1);
    ASSERT_NE(set.End(), iter2);
    ASSERT_NE(set.End(), iter3);
    ASSERT_EQ(set.End(), set.Find(nullptr));

    ASSERT_FALSE(set.Erase(nullptr));
    ASSERT_EQ(3u, set.Size());

    ASSERT_TRUE(set.Erase(iter2));

    {
        iter1 = std::find_if(set.begin(), set.end(), [&](const KeyType& key) { return key.Get() == rawPtr1; });
        iter2 = std::find_if(set.begin(), set.end(), [&](const KeyType& key) { return key.Get() == rawPtr2; });
        iter3 = std::find_if(set.begin(), set.end(), [&](const KeyType& key) { return key.Get() == rawPtr3; });

        ASSERT_EQ(2u, set.Size());
        ASSERT_NE(set.End(), iter1);
        ASSERT_EQ(set.End(), iter2);
        ASSERT_NE(set.End(), iter3);
        ASSERT_EQ(set.End(), set.Find(nullptr));
    }
}
