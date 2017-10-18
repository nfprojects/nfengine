/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for HashHashSet (when used with other containers)
 */

#include "PCH.hpp"
#include "nfCommon/Containers/HashSet.hpp"
#include "nfCommon/Containers/String.hpp"
#include "nfCommon/Containers/SharedPtr.hpp"
#include "nfCommon/Containers/DynArray.hpp"

#include "TestClasses.hpp"

using namespace NFE;
using namespace NFE::Common;

TEST(HashSet, HashSetOfStrings)
{
    HashSet<String> set;

    ASSERT_NE(set.End(), set.Insert("aaa").iterator);
    ASSERT_NE(set.End(), set.Insert("bbb").iterator);

    ASSERT_NE(set.End(), set.Find("aaa"));
    ASSERT_NE(set.End(), set.Find("bbb"));
    ASSERT_EQ(set.End(), set.Find("ccc"));

    ASSERT_TRUE(set.Erase("aaa"));
    ASSERT_TRUE(set.Erase("bbb"));
    ASSERT_FALSE(set.Erase("ccc"));
}

TEST(HashSet, HashSetOfDynArrays)
{
    using KeyType = DynArray<int>;
    HashSet<KeyType> set;

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

TEST(HashSet, HashSetOfSharedPtrs)
{
    using KeyType = SharedPtr<int>;
    HashSet<KeyType> set;

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
