/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for StringView
 */

#include "PCH.hpp"
#include "Engine/Common/Containers/StringView.hpp"


using namespace NFE;
using namespace NFE::Common;

TEST(StringView, Empty)
{
    StringView stringView;

    ASSERT_TRUE(stringView.Empty());
    ASSERT_EQ(0u, stringView.Length());
}

TEST(StringView, ConstString)
{
    const char* testString = "test";
    StringView stringView(testString);

    ASSERT_FALSE(stringView.Empty());
    ASSERT_EQ(4u, stringView.Length());

    EXPECT_EQ('t', stringView[0]);
    EXPECT_EQ('e', stringView[1]);
    EXPECT_EQ('s', stringView[2]);
    EXPECT_EQ('t', stringView[3]);
}

TEST(StringView, ConstStringLength)
{
    const char* testString = "testigbwugbw9gtbw9bg";
    StringView stringView(testString, 4);

    ASSERT_FALSE(stringView.Empty());
    ASSERT_EQ(4u, stringView.Length());

    EXPECT_EQ('t', stringView[0]);
    EXPECT_EQ('e', stringView[1]);
    EXPECT_EQ('s', stringView[2]);
    EXPECT_EQ('t', stringView[3]);
}

TEST(StringView, CompareEqual_DifferentPointer)
{
    const char* testString = "testtest";
    StringView stringViewA(testString, 4);
    StringView stringViewB(testString + 4, 4);
    // make sure that pointers are different
    ASSERT_NE(stringViewA.Data(), stringViewB.Data());

    EXPECT_TRUE(stringViewA == stringViewB);
    EXPECT_TRUE(stringViewB == stringViewA);
    EXPECT_TRUE(stringViewA >= stringViewB);
    EXPECT_TRUE(stringViewB >= stringViewA);
    EXPECT_TRUE(stringViewA <= stringViewB);
    EXPECT_TRUE(stringViewB <= stringViewA);

    EXPECT_FALSE(stringViewA != stringViewB);
    EXPECT_FALSE(stringViewB != stringViewA);
    EXPECT_FALSE(stringViewA < stringViewB);
    EXPECT_FALSE(stringViewB < stringViewA);
    EXPECT_FALSE(stringViewA > stringViewB);
    EXPECT_FALSE(stringViewB > stringViewA);
}

TEST(StringView, CompareEqual_SamePointer)
{
    const char* testString = "test";
    StringView stringViewA(testString);
    StringView stringViewB(testString);
    // make sure that pointers are the same
    ASSERT_EQ(stringViewA.Data(), stringViewB.Data());

    EXPECT_TRUE(stringViewA == stringViewB);
    EXPECT_TRUE(stringViewB == stringViewA);
    EXPECT_TRUE(stringViewA >= stringViewB);
    EXPECT_TRUE(stringViewB >= stringViewA);
    EXPECT_TRUE(stringViewA <= stringViewB);
    EXPECT_TRUE(stringViewB <= stringViewA);

    EXPECT_FALSE(stringViewA != stringViewB);
    EXPECT_FALSE(stringViewB != stringViewA);
    EXPECT_FALSE(stringViewA < stringViewB);
    EXPECT_FALSE(stringViewB < stringViewA);
    EXPECT_FALSE(stringViewA > stringViewB);
    EXPECT_FALSE(stringViewB > stringViewA);
}

TEST(StringView, CompareDifferent_SameLength)
{
    StringView stringViewA("abcd");
    StringView stringViewB("abxz");
    // make sure that pointers are different
    ASSERT_NE(stringViewA.Data(), stringViewB.Data());

    EXPECT_FALSE(stringViewA == stringViewB);
    EXPECT_FALSE(stringViewB == stringViewA);
    EXPECT_FALSE(stringViewA >= stringViewB);
    EXPECT_TRUE(stringViewB >= stringViewA);
    EXPECT_TRUE(stringViewA <= stringViewB);
    EXPECT_FALSE(stringViewB <= stringViewA);

    EXPECT_TRUE(stringViewA != stringViewB);
    EXPECT_TRUE(stringViewB != stringViewA);
    EXPECT_TRUE(stringViewA < stringViewB);
    EXPECT_FALSE(stringViewB < stringViewA);
    EXPECT_FALSE(stringViewA > stringViewB);
    EXPECT_TRUE(stringViewB > stringViewA);
}

TEST(StringView, CompareDifferentLength)
{
    StringView stringViewA("abcd");
    StringView stringViewB("abcde");

    EXPECT_FALSE(stringViewA == stringViewB);
    EXPECT_FALSE(stringViewB == stringViewA);
    EXPECT_FALSE(stringViewA >= stringViewB);
    EXPECT_TRUE(stringViewB >= stringViewA);
    EXPECT_TRUE(stringViewA <= stringViewB);
    EXPECT_FALSE(stringViewB <= stringViewA);

    EXPECT_TRUE(stringViewA != stringViewB);
    EXPECT_TRUE(stringViewB != stringViewA);
    EXPECT_TRUE(stringViewA < stringViewB);
    EXPECT_FALSE(stringViewB < stringViewA);
    EXPECT_FALSE(stringViewA > stringViewB);
    EXPECT_TRUE(stringViewB > stringViewA);
}

TEST(StringView, FindFirst_Char)
{
    StringView stringView("abcdae");

    EXPECT_EQ(0u, stringView.FindFirst('a'));
    EXPECT_EQ(1u, stringView.FindFirst('b'));
    EXPECT_EQ(2u, stringView.FindFirst('c'));
    EXPECT_EQ(3u, stringView.FindFirst('d'));
    EXPECT_EQ(5u, stringView.FindFirst('e'));

    EXPECT_EQ(StringView::END(), stringView.FindFirst('Z'));
    EXPECT_EQ(StringView::END(), stringView.FindFirst(' '));
}

TEST(StringView, FindLast_End)
{
    StringView stringView("abcdae");

    EXPECT_EQ(4u, stringView.FindLast('a'));
    EXPECT_EQ(1u, stringView.FindLast('b'));
    EXPECT_EQ(2u, stringView.FindLast('c'));
    EXPECT_EQ(3u, stringView.FindLast('d'));
    EXPECT_EQ(5u, stringView.FindLast('e'));

    EXPECT_EQ(StringView::END(), stringView.FindLast('Z'));
    EXPECT_EQ(StringView::END(), stringView.FindLast(' '));
}

TEST(StringView, FindFirst_SubString)
{
    StringView stringView("abcdaeabcdae");

    EXPECT_EQ(0u, stringView.FindFirst(StringView("a")));
    EXPECT_EQ(1u, stringView.FindFirst(StringView("b")));
    EXPECT_EQ(2u, stringView.FindFirst(StringView("c")));
    EXPECT_EQ(3u, stringView.FindFirst(StringView("d")));
    EXPECT_EQ(5u, stringView.FindFirst(StringView("e")));

    EXPECT_EQ(0u, stringView.FindFirst(StringView("ab")));
    EXPECT_EQ(0u, stringView.FindFirst(StringView("abc")));
    EXPECT_EQ(0u, stringView.FindFirst(StringView("abcd")));
    EXPECT_EQ(0u, stringView.FindFirst(StringView("abcda")));
    EXPECT_EQ(0u, stringView.FindFirst(StringView("abcdae")));
    EXPECT_EQ(1u, stringView.FindFirst(StringView("bcdae")));
    EXPECT_EQ(2u, stringView.FindFirst(StringView("cdae")));
    EXPECT_EQ(3u, stringView.FindFirst(StringView("dae")));
    EXPECT_EQ(4u, stringView.FindFirst(StringView("ae")));

    EXPECT_EQ(StringView::END(), stringView.FindFirst(StringView("aba")));
    EXPECT_EQ(StringView::END(), stringView.FindFirst(StringView("aeG")));
    EXPECT_EQ(StringView::END(), stringView.FindFirst(StringView("ae+")));
    EXPECT_EQ(StringView::END(), stringView.FindFirst(StringView("Z")));
    EXPECT_EQ(StringView::END(), stringView.FindFirst(StringView("")));
    EXPECT_EQ(StringView::END(), stringView.FindFirst(StringView("abcdaeabcdaeZ")));
    EXPECT_EQ(StringView::END(), stringView.FindFirst(StringView("0000000000000")));
}

TEST(StringView, FindLast_SubString)
{
    StringView stringView("abcdaeabcdae");

    EXPECT_EQ(10u, stringView.FindLast(StringView("a")));
    EXPECT_EQ(7u, stringView.FindLast(StringView("b")));
    EXPECT_EQ(8u, stringView.FindLast(StringView("c")));
    EXPECT_EQ(9u, stringView.FindLast(StringView("d")));
    EXPECT_EQ(11u, stringView.FindLast(StringView("e")));

    EXPECT_EQ(6u, stringView.FindLast(StringView("ab")));
    EXPECT_EQ(6u, stringView.FindLast(StringView("abc")));
    EXPECT_EQ(6u, stringView.FindLast(StringView("abcd")));
    EXPECT_EQ(6u, stringView.FindLast(StringView("abcda")));
    EXPECT_EQ(6u, stringView.FindLast(StringView("abcdae")));
    EXPECT_EQ(7u, stringView.FindLast(StringView("bcdae")));
    EXPECT_EQ(8u, stringView.FindLast(StringView("cdae")));
    EXPECT_EQ(9u, stringView.FindLast(StringView("dae")));
    EXPECT_EQ(10u, stringView.FindLast(StringView("ae")));

    EXPECT_EQ(StringView::END(), stringView.FindLast(StringView("ae+")));
    EXPECT_EQ(StringView::END(), stringView.FindLast(StringView("Z")));
    EXPECT_EQ(StringView::END(), stringView.FindLast(StringView("")));
    EXPECT_EQ(StringView::END(), stringView.FindLast(StringView("abcdaeabcdaeZ")));
    EXPECT_EQ(StringView::END(), stringView.FindLast(StringView("0000000000000")));
}

// TODO range test
