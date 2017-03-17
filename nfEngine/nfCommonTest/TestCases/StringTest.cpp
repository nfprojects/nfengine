/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for String.
 */

#include "PCH.hpp"
#include "nfCommon/Containers/String.hpp"


using namespace NFE;
using namespace NFE::Common;


#define TEST_STRING_SHORT "A"

// a string that won't fit internal buffer
#define TEST_STRING_LONG "BBBBBBBBBBBB"
#define TEST_STRING_LONG_LEN 12u

TEST(String, Constructor_Default)
{
    String string;

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("", string.Str());
}

TEST(String, Constructor_Char)
{
    String string('a');

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("a", string.Str());
}

TEST(String, Constructor_View_Short)
{
    const StringView view("a");

    String string(view);

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("a", string.Str());
}

TEST(String, Constructor_View_Long)
{
    const StringView view(TEST_STRING_LONG);

    String string(view);

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, string.Length());
    ASSERT_LE(TEST_STRING_LONG_LEN + 1u, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ(TEST_STRING_LONG, string.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, MoveConstructor_Empty)
{
    String string;
    const String other(std::move(string));

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("", string.Str());

    ASSERT_TRUE(other.Empty());
    ASSERT_EQ(0u, other.Length());
    ASSERT_NE(nullptr, other.Str());
    ASSERT_STREQ("", other.Str());
}

TEST(String, MoveConstructor_Short)
{
    String string("a");
    const String other(std::move(string));

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("", string.Str());

    ASSERT_FALSE(other.Empty());
    ASSERT_EQ(1u, other.Length());
    ASSERT_NE(nullptr, other.Str());
    ASSERT_STREQ("a", other.Str());
}

TEST(String, MoveConstructor_Long)
{
    String string(TEST_STRING_LONG);
    const String other(std::move(string));

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("", string.Str());

    ASSERT_FALSE(other.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, other.Length());
    ASSERT_NE(nullptr, other.Str());
    ASSERT_STREQ(TEST_STRING_LONG, other.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, MoveAssignment_Empty)
{
    String string;
    String other("a");

    other = std::move(string);

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("", string.Str());

    ASSERT_TRUE(other.Empty());
    ASSERT_EQ(0u, other.Length());
    ASSERT_NE(nullptr, other.Str());
    ASSERT_STREQ("", other.Str());
}

TEST(String, MoveAssignment_Short)
{
    String string("a");
    String other("a");

    other = std::move(string);

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("", string.Str());

    ASSERT_FALSE(other.Empty());
    ASSERT_EQ(1u, other.Length());
    ASSERT_NE(nullptr, other.Str());
    ASSERT_STREQ("a", other.Str());
}

TEST(String, MoveAssignment_Long)
{
    String string(TEST_STRING_LONG);
    String other("a");

    other = std::move(string);

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("", string.Str());

    ASSERT_FALSE(other.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, other.Length());
    ASSERT_NE(nullptr, other.Str());
    ASSERT_STREQ(TEST_STRING_LONG, other.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Constructor_Copy_Empty)
{
    String string;
    String copy(string);

    ASSERT_TRUE(copy.Empty());
    ASSERT_EQ(0u, copy.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, copy.Capacity());
    ASSERT_NE(nullptr, copy.Str());
    ASSERT_STREQ("", copy.Str());
}

TEST(String, Constructor_Copy_Short)
{
    String string('a');
    String copy(string);

    ASSERT_FALSE(copy.Empty());
    ASSERT_EQ(1u, copy.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, copy.Capacity());
    ASSERT_NE(nullptr, copy.Str());
    ASSERT_STREQ("a", copy.Str());
}

TEST(String, Constructor_Copy_Long)
{
    const StringView view(TEST_STRING_LONG);

    String string(view);
    String copy(string);

    ASSERT_FALSE(copy.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, copy.Length());
    ASSERT_LE(TEST_STRING_LONG_LEN + 1u, copy.Capacity());
    ASSERT_NE(nullptr, copy.Str());
    ASSERT_STREQ(TEST_STRING_LONG, copy.Str());
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

//////////////////////////////////////////////////////////////////////////

TEST(String, Reserve_Small)
{
    const uint32 targetSize = 8;

    String string('a');

    ASSERT_TRUE(string.Reserve(targetSize));

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
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
    ASSERT_EQ(1u, string.Length());
    ASSERT_LE(targetSize, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("a", string.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Assign_Char)
{
    String string('a');
    string = 'b';

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("b", string.Str());
}

TEST(String, Assign_String)
{
    String string('a');
    string = String("b");

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("b", string.Str());
}

TEST(String, Assign_String_Self)
{
    String string('a');
    string = string;

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("a", string.Str());
}

TEST(String, Assign_StringView_Short)
{
    String string('a');
    string = StringView("b");

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("b", string.Str());
}

TEST(String, Assign_StringView_Long)
{
    String string('a');
    string = StringView(TEST_STRING_LONG);

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, string.Length());
    ASSERT_LE(TEST_STRING_LONG_LEN + 1u, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ(TEST_STRING_LONG, string.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Append_Char_Short)
{
    String string('a');
    string += 'b';

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(2u, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("ab", string.Str());
}

TEST(String, Append_Char_Long)
{
    String string("aaaaaaaaaaa");
    ASSERT_EQ(11u, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());

    string += 'b';

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(12u, string.Length());
    ASSERT_LE(13u, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("aaaaaaaaaaab", string.Str());
}

TEST(String, Append_String)
{
    // TODO
}

TEST(String, Append_StringView)
{
    String string('a');
    string += StringView("bcd");

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(4u, string.Length());
    ASSERT_EQ(NFE_INTERNAL_STRING_LENGTH + 1, string.Capacity());
    ASSERT_NE(nullptr, string.Str());
    ASSERT_STREQ("abcd", string.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Concatenate_StringView_StringView_Short)
{
    const StringView a("a");
    const StringView b("b");
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_NE(nullptr, c.Str());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_StringView_StringView_Long0)
{
    const StringView a("a");
    const StringView b("bbbbbbbbbbb");
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(12u, c.Length());
    ASSERT_NE(nullptr, c.Str());
    ASSERT_STREQ("abbbbbbbbbbb", c.Str());
}

TEST(String, Concatenate_StringView_StringView_Long1)
{
    const StringView a("bbbbbbbbbbb");
    const StringView b("a");
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(12u, c.Length());
    ASSERT_NE(nullptr, c.Str());
    ASSERT_STREQ("bbbbbbbbbbba", c.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Insert_Char)
{
    // TODO
}

TEST(String, Insert_String)
{
    // TODO
}

TEST(String, Insert_StringView)
{
    // TODO
}

TEST(String, Erase)
{
    // TODO
}

TEST(String, Replace_Char)
{
    // TODO
}

TEST(String, Replace_String)
{
    // TODO
}

TEST(String, Replace_StringView)
{
    // TODO
}