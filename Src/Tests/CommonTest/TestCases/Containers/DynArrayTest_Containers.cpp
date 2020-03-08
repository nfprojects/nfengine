/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for HashHashSet (when used with other containers)
 */

#include "PCH.hpp"
#include "Engine/Common/Containers/DynArray.hpp"
#include "Engine/Common/Containers/String.hpp"
#include "Engine/Common/Containers/SharedPtr.hpp"


using namespace NFE;
using namespace NFE::Common;


// Note: use long strings so they must be allocated dynamically
namespace
{
const char* strA = "aaaaaaaaaaaaaaaaaaaaaaaaaa";
const char* strB = "bbbbbbbbbbbbbbbbbbbbbbbbbb";
const char* strC = "cccccccccccccccccccccccccc";
const char* strD = "dddddddddddddddddddddddddd";

} // namespace

//////////////////////////////////////////////////////////////////////////

TEST(DynArray, DynArrayOfStrings_Simple)
{
    DynArray<String> array;

    array.PushBack(strA);
    array.PushBack(strB);
    array.PushBack(strC);

    ASSERT_EQ(3u, array.Size());
    ASSERT_NE(array.End(), array.Find(strA));
    ASSERT_NE(array.End(), array.Find(strB));
    ASSERT_NE(array.End(), array.Find(strC));
    ASSERT_EQ(array.End(), array.Find("d"));
}

TEST(DynArray, DynArrayOfStrings_Erase)
{
    DynArray<String> array;
    // Note: use long strings so they must be allocated dynamically
    array.PushBack(strA);
    array.PushBack(strB);
    array.PushBack(strC);

    DynArray<String> expectedArray;
    expectedArray.PushBack(strA);
    expectedArray.PushBack(strC);

    array.Erase(array.Begin() + 1);
    ASSERT_EQ(2u, array.Size());
    ASSERT_EQ(expectedArray, array);

    ASSERT_EQ(array.End(), array.Find(strB));
}

TEST(DynArray, DynArrayOfStrings_Insert)
{
    DynArray<String> array;
    array.PushBack(strA);
    array.PushBack(strB);
    array.PushBack(strC);

    DynArray<String> expectedArray;
    expectedArray.PushBack(strA);
    expectedArray.PushBack(strD);
    expectedArray.PushBack(strB);
    expectedArray.PushBack(strC);

    array.InsertAt(1, strD);
    ASSERT_EQ(4u, array.Size());
    ASSERT_EQ(expectedArray, array);

    ASSERT_NE(array.End(), array.Find(strD));
}

//////////////////////////////////////////////////////////////////////////

TEST(DynArray, DynArrayOfUniquePtrs_Simple)
{
    DynArray<UniquePtr<int>> array;

    array.PushBack(MakeUniquePtr<int>(1));
    array.PushBack(MakeUniquePtr<int>(2));
    array.PushBack(MakeUniquePtr<int>(3));

    ASSERT_EQ(3u, array.Size());
    EXPECT_NE(array.End(), std::find_if(array.Begin(), array.End(), [](const UniquePtr<int>& x) { return *x == 1; }));
    EXPECT_EQ(array.End(), std::find_if(array.Begin(), array.End(), [](const UniquePtr<int>& x) { return *x == 1234; }));
}

TEST(DynArray, DynArrayOfUniquePtrs_Erase)
{
    DynArray<UniquePtr<int>> array;

    array.PushBack(MakeUniquePtr<int>(1));
    array.PushBack(MakeUniquePtr<int>(2));
    array.PushBack(MakeUniquePtr<int>(3));

    ASSERT_EQ(3u, array.Size());
    EXPECT_NE(array.End(), std::find_if(array.Begin(), array.End(), [](const UniquePtr<int>& x) { return *x == 1; }));
    EXPECT_NE(array.End(), std::find_if(array.Begin(), array.End(), [](const UniquePtr<int>& x) { return *x == 2; }));
    EXPECT_NE(array.End(), std::find_if(array.Begin(), array.End(), [](const UniquePtr<int>& x) { return *x == 3; }));

    array.Erase(array.begin() + 1);

    ASSERT_EQ(2u, array.Size());
    EXPECT_NE(array.End(), std::find_if(array.Begin(), array.End(), [](const UniquePtr<int>& x) { return *x == 1; }));
    EXPECT_EQ(array.End(), std::find_if(array.Begin(), array.End(), [](const UniquePtr<int>& x) { return *x == 2; }));
    EXPECT_NE(array.End(), std::find_if(array.Begin(), array.End(), [](const UniquePtr<int>& x) { return *x == 3; }));
}

//////////////////////////////////////////////////////////////////////////

TEST(DynArray, DynArrayOfDynArrays_Simple)
{
    DynArray<DynArray<int>> array;

    array.PushBack(DynArray<int>{1});
    array.PushBack(DynArray<int>{2, 3});
    array.PushBack(DynArray<int>{4, 5, 6});

    ASSERT_EQ(3u, array.Size());
    ASSERT_NE(array.End(), array.Find(DynArray<int>{1}));
    ASSERT_NE(array.End(), array.Find(DynArray<int>{2, 3}));
    ASSERT_NE(array.End(), array.Find(DynArray<int>{4, 5, 6}));
    ASSERT_EQ(array.End(), array.Find(DynArray<int>{1, 2, 3}));
}

TEST(DynArray, DynArrayOfDynArrays_CopyConstructor)
{
    DynArray<DynArray<int>> array;

    array.PushBack(DynArray<int>{1});
    array.PushBack(DynArray<int>{2, 3});
    array.PushBack(DynArray<int>{4, 5, 6});

    DynArray<DynArray<int>> arrayCopy(array);
    ASSERT_EQ(array, arrayCopy);

    ASSERT_EQ(3u, array.Size());
    ASSERT_EQ(3u, arrayCopy.Size());
    ASSERT_NE(arrayCopy.End(), arrayCopy.Find(DynArray<int>{1}));
    ASSERT_NE(arrayCopy.End(), arrayCopy.Find(DynArray<int>{2, 3}));
    ASSERT_NE(arrayCopy.End(), arrayCopy.Find(DynArray<int>{4, 5, 6}));
    ASSERT_EQ(arrayCopy.End(), arrayCopy.Find(DynArray<int>{1, 2, 3}));
}

TEST(DynArray, DynArrayOfDynArrays_MoveConstructor)
{
    DynArray<DynArray<int>> array;

    array.PushBack(DynArray<int>{1});
    array.PushBack(DynArray<int>{2, 3});
    array.PushBack(DynArray<int>{4, 5, 6});

    DynArray<DynArray<int>> arrayCopy(std::move(array));
    ASSERT_NE(array, arrayCopy);

    ASSERT_EQ(0u, array.Size());
    ASSERT_EQ(3u, arrayCopy.Size());
    ASSERT_NE(arrayCopy.End(), arrayCopy.Find(DynArray<int>{1}));
    ASSERT_NE(arrayCopy.End(), arrayCopy.Find(DynArray<int>{2, 3}));
    ASSERT_NE(arrayCopy.End(), arrayCopy.Find(DynArray<int>{4, 5, 6}));
    ASSERT_EQ(arrayCopy.End(), arrayCopy.Find(DynArray<int>{1, 2, 3}));
}

TEST(DynArray, DynArrayOfDynArrays_Erase)
{
    DynArray<DynArray<int>> array;

    array.PushBack(DynArray<int>{1});
    array.PushBack(DynArray<int>{2, 3});
    array.PushBack(DynArray<int>{4, 5, 6});

    array.Erase(array.Begin() + 1);

    ASSERT_EQ(2u, array.Size());
    ASSERT_NE(array.End(), array.Find(DynArray<int>{1}));
    ASSERT_EQ(array.End(), array.Find(DynArray<int>{2, 3}));
    ASSERT_NE(array.End(), array.Find(DynArray<int>{4, 5, 6}));
    ASSERT_EQ(array.End(), array.Find(DynArray<int>{1, 2, 3}));
}
