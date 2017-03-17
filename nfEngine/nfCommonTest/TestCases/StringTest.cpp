/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for String.
 */

#include "PCH.hpp"
#include "nfCommon/Containers/String.hpp"


using namespace NFE;
using namespace NFE::Common;

TEST(String, Constructor_Default)
{
    String string;

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("", string.Str());
}

TEST(String, Constructor_Char)
{
    String string('a');

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("a", string.Str());
}

TEST(String, Constructor_View_Short)
{
    const StringView view("a");

    String string(view);

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("a", string.Str());
}

TEST(String, Constructor_View_Long)
{
    const StringView view("qwertyuiop[]asdfghjkl;");

    String string(view);

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(22, string.Length());
    ASSERT_LE(23u, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("qwertyuiop[]asdfghjkl;", string.Str());
}

TEST(String, Constructor_Copy_Empty)
{
    String string;
    String copy(string);

    ASSERT_TRUE(copy.Empty());
    ASSERT_EQ(0, copy.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, copy.Capacity());
    ASSERT_NE(nullptr, copy.Str());
    ASSERT_STREQ("", copy.Str());
}

TEST(String, Constructor_Copy_Short)
{
    String string('a');
    String copy(string);

    ASSERT_FALSE(copy.Empty());
    ASSERT_EQ(1, copy.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, copy.Capacity());
    ASSERT_NE(nullptr, copy.Str());
    ASSERT_STREQ("a", copy.Str());
}

TEST(String, Constructor_Copy_Long)
{
    const StringView view("qwertyuiop[]asdfghjkl;");

    String string(view);
    String copy(string);

    ASSERT_FALSE(copy.Empty());
    ASSERT_EQ(22, copy.Length());
    ASSERT_LE(23u, copy.Capacity());
    ASSERT_NE(nullptr, copy.Str());
    ASSERT_STREQ("qwertyuiop[]asdfghjkl;", copy.Str());
}

TEST(String, Constructor_Copy_ExternalToInternal)
{
    const uint32 targetSize = 1024;

    String string('a');
    ASSERT_TRUE(string.Reserve(targetSize));
    ASSERT_LE(targetSize, string.Capacity());

    String copy(string);

    ASSERT_FALSE(copy.Empty());
    ASSERT_EQ(1, copy.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, copy.Capacity());
    ASSERT_NE(nullptr, copy.Str());
    ASSERT_STREQ("a", copy.Str());
}

TEST(String, Reserve_Small)
{
    const uint32 targetSize = 8;

    String string('a');

    ASSERT_TRUE(string.Reserve(targetSize));

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1, string.Length());
    ASSERT_LE(targetSize, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("a", string.Str());
}

TEST(String, Reserve_Big)
{
    const uint32 targetSize = 1024;

    String string('a');

    ASSERT_TRUE(string.Reserve(targetSize));

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1, string.Length());
    ASSERT_LE(targetSize, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("a", string.Str());
}