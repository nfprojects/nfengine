/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for StringView
 */

#include "PCH.hpp"
#include "nfCommon/Containers/StringView.hpp"


using namespace NFE;
using namespace NFE::Common;

TEST(StringView, Empty)
{
    StringView stringView;
    
    ASSERT_TRUE(stringView.Empty());
    ASSERT_EQ(0, stringView.Length());
}

TEST(StringView, ConstString)
{
    const char* testString = "test";
    StringView stringView(testString);

    ASSERT_FALSE(stringView.Empty());
    ASSERT_EQ(4, stringView.Length());

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
    ASSERT_EQ(4, stringView.Length());

    EXPECT_EQ('t', stringView[0]);
    EXPECT_EQ('e', stringView[1]);
    EXPECT_EQ('s', stringView[2]);
    EXPECT_EQ('t', stringView[3]);
}
