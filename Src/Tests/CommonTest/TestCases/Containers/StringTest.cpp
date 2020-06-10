/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for String.
 */

#include "PCH.hpp"
#include "Engine/Common/Containers/String.hpp"


using namespace NFE;
using namespace NFE::Common;


#define TEST_STRING_SHORT "A"

// a string that fits internal buffer (but any additional character won't)
#define TEST_STRING_MID_A "AAAAAAAAAAA"
#define TEST_STRING_MID_B "BBBBBBBBBBB"
#define TEST_STRING__LEN 11u

// a string that won't fit internal buffer
#define TEST_STRING_LONG_A "AAAAAAAAAAAA"
#define TEST_STRING_LONG_B "BBBBBBBBBBBB"
#define TEST_STRING_LONG_LEN 12u

TEST(String, Constructor_Default)
{
    String string;

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, string.Capacity());
    ASSERT_STREQ("", string.Str());
}

TEST(String, Constructor_Char)
{
    String string('a');

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, string.Capacity());
    ASSERT_STREQ("a", string.Str());
}

TEST(String, Constructor_View_Short)
{
    const StringView view("a");

    String string(view);

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, string.Capacity());
    ASSERT_STREQ("a", string.Str());
}

TEST(String, Constructor_View_Long)
{
    const StringView view(TEST_STRING_LONG_A);

    String string(view);

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, string.Length());
    ASSERT_LE(TEST_STRING_LONG_LEN + 1u, string.Capacity());
    ASSERT_STREQ(TEST_STRING_LONG_A, string.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, MoveConstructor_Empty)
{
    String string;
    const String other(std::move(string));

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_STREQ("", string.Str());

    ASSERT_TRUE(other.Empty());
    ASSERT_EQ(0u, other.Length());
    ASSERT_STREQ("", other.Str());
}

TEST(String, MoveConstructor_Short)
{
    String string("a");
    const String other(std::move(string));

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_STREQ("", string.Str());

    ASSERT_FALSE(other.Empty());
    ASSERT_EQ(1u, other.Length());
    ASSERT_STREQ("a", other.Str());
}

TEST(String, MoveConstructor_Long)
{
    String string(TEST_STRING_LONG_A);
    const String other(std::move(string));

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_STREQ("", string.Str());

    ASSERT_FALSE(other.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, other.Length());
    ASSERT_STREQ(TEST_STRING_LONG_A, other.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, MoveAssignment_Empty)
{
    String string;
    String other("a");

    other = std::move(string);

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_STREQ("", string.Str());

    ASSERT_TRUE(other.Empty());
    ASSERT_EQ(0u, other.Length());
    ASSERT_STREQ("", other.Str());
}

TEST(String, MoveAssignment_Short)
{
    String string("a");
    String other("a");

    other = std::move(string);

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_STREQ("", string.Str());

    ASSERT_FALSE(other.Empty());
    ASSERT_EQ(1u, other.Length());
    ASSERT_STREQ("a", other.Str());
}

TEST(String, MoveAssignment_Long)
{
    String string(TEST_STRING_LONG_A);
    String other("a");

    other = std::move(string);

    ASSERT_TRUE(string.Empty());
    ASSERT_EQ(0u, string.Length());
    ASSERT_STREQ("", string.Str());

    ASSERT_FALSE(other.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, other.Length());
    ASSERT_STREQ(TEST_STRING_LONG_A, other.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Constructor_Copy_Empty)
{
    String string;
    String copy(string);

    ASSERT_TRUE(copy.Empty());
    ASSERT_EQ(0u, copy.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, copy.Capacity());
    ASSERT_STREQ("", copy.Str());
}

TEST(String, Constructor_Copy_Short)
{
    String string('a');
    String copy(string);

    ASSERT_FALSE(copy.Empty());
    ASSERT_EQ(1u, copy.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, copy.Capacity());
    ASSERT_STREQ("a", copy.Str());
}

TEST(String, Constructor_Copy_Long)
{
    const StringView view(TEST_STRING_LONG_A);

    String string(view);
    String copy(string);

    ASSERT_FALSE(copy.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, copy.Length());
    ASSERT_LE(TEST_STRING_LONG_LEN + 1u, copy.Capacity());
    ASSERT_STREQ(TEST_STRING_LONG_A, copy.Str());
}

TEST(String, Constructor_Copy_ExternalToInternal)
{
    const uint32 targetSize = 1024;

    String string('a');
    ASSERT_TRUE(string.Reserve(targetSize));
    ASSERT_LE(targetSize, string.Capacity());

    String copy(string);

    ASSERT_FALSE(copy.Empty());
    ASSERT_EQ(1u, copy.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, copy.Capacity());
    ASSERT_STREQ("a", copy.Str());
}

TEST(String, Constructor_FixedArray)
{
    const char fixedArray[6] = { 'f', 'o', 'o', 'b', 'a', 'r' };
    const String string = String::ConstructFromFixedArray(fixedArray, 3);

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(3u, string.Length());
    ASSERT_STREQ("foo", string.Str());
}

TEST(String, Constructor_Printf)
{
    const String string = String::Printf("%u %s", 1, "foo");

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(5u, string.Length());
    ASSERT_STREQ("1 foo", string.Str());
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
    ASSERT_STREQ("a", string.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Assign_Char)
{
    String string('a');
    string = 'b';

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, string.Capacity());
    ASSERT_STREQ("b", string.Str());
}

TEST(String, Assign_String)
{
    String string('a');
    string = String("b");

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_STREQ("b", string.Str());
}

TEST(String, Assign_String_Self)
{
    String string('a');
    string = string;

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_STREQ("a", string.Str());
}

TEST(String, Assign_StringView_Short)
{
    String string('a');
    string = StringView("b");

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(1u, string.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, string.Capacity());
    ASSERT_STREQ("b", string.Str());
}

TEST(String, Assign_StringView_Long)
{
    String string('a');
    string = StringView(TEST_STRING_LONG_A);

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN, string.Length());
    ASSERT_LE(TEST_STRING_LONG_LEN + 1u, string.Capacity());
    ASSERT_STREQ(TEST_STRING_LONG_A, string.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Append_Char_Short)
{
    String string('a');
    string += 'b';

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(2u, string.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, string.Capacity());
    ASSERT_STREQ("ab", string.Str());
}

TEST(String, Append_Char_Long)
{
    String string(TEST_STRING_MID_A);
    ASSERT_EQ(11u, string.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, string.Capacity());

    string += 'b';

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(12u, string.Length());
    ASSERT_LE(13u, string.Capacity());
    ASSERT_STREQ(TEST_STRING_MID_A "b", string.Str());
}

TEST(String, Append_StringView)
{
    String string('a');
    string += StringView("bcd");

    ASSERT_FALSE(string.Empty());
    ASSERT_EQ(4u, string.Length());
    ASSERT_EQ(String::MaxInternalLength + 1, string.Capacity());
    ASSERT_STREQ("abcd", string.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Concatenate_String_String_Short)
{
    const String a("a");
    const String b("b");
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_String_CString_Short)
{
    const String a("a");
    const char* b = "b";
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_CString_String_Short)
{
    const char* a = "a";
    const String b("b");
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_StringView_StringView_Short)
{
    const StringView a("a");
    const StringView b("b");
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_StringView_CString_Short)
{
    const StringView a("a");
    const char* b = "b";
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_CString_StringView_Short)
{
    const char* a = "a";
    const StringView b("b");
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_StringView_StringView_Long0)
{
    const StringView a("a");
    const StringView b(TEST_STRING_MID_B);
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(12u, c.Length());
    ASSERT_STREQ("a" TEST_STRING_MID_B, c.Str());
}

TEST(String, Concatenate_StringView_StringView_Long1)
{
    const StringView a(TEST_STRING_MID_A);
    const StringView b("b");
    const String c = a + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(12u, c.Length());
    ASSERT_STREQ(TEST_STRING_MID_A "b", c.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Concatenate_MoveString_StringView_Short)
{
    String a("a");
    const StringView b("b");
    const String c = std::move(a) + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_MoveString_StringView_Long0)
{
    String a(TEST_STRING_LONG_A);
    const StringView b("b");
    const String c = std::move(a) + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + 1, c.Length());
    ASSERT_STREQ(TEST_STRING_LONG_A "b", c.Str());
}

TEST(String, Concatenate_MoveString_StringView_Long1)
{
    String a("a");
    const StringView b(TEST_STRING_LONG_B);
    const String c = std::move(a) + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + 1, c.Length());
    ASSERT_STREQ("a" TEST_STRING_LONG_B, c.Str());
}

TEST(String, Concatenate_MoveString_StringView_Long2)
{
    String a(TEST_STRING_LONG_A);
    const StringView b(TEST_STRING_LONG_B);
    const String c = std::move(a) + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + TEST_STRING_LONG_LEN, c.Length());
    ASSERT_STREQ(TEST_STRING_LONG_A TEST_STRING_LONG_B, c.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Concatenate_StringView_MoveString_Short)
{
    const StringView a("a");
    String b("b");
    const String c = a + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_StringView_MoveString_Long0)
{
    const StringView a(TEST_STRING_LONG_A);
    String b("b");
    const String c = a + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + 1, c.Length());
    ASSERT_STREQ(TEST_STRING_LONG_A "b", c.Str());
}

TEST(String, Concatenate_StringView_MoveString_Long1)
{
    const StringView a("a");
    String b(TEST_STRING_LONG_B);
    const String c = a + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + 1, c.Length());
    ASSERT_STREQ("a" TEST_STRING_LONG_B, c.Str());
}

TEST(String, Concatenate_StringView_MoveString_Long2)
{
    const StringView a(TEST_STRING_LONG_A);
    String b(TEST_STRING_LONG_B);
    const String c = a + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + TEST_STRING_LONG_LEN, c.Length());
    ASSERT_STREQ(TEST_STRING_LONG_A TEST_STRING_LONG_B, c.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Concatenate_MoveString_MoveString_Short)
{
    String a("a");
    String b("b");
    const String c = std::move(a) + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_MoveString_MoveString_Long0)
{
    String a(TEST_STRING_LONG_A);
    String b("b");
    const String c = std::move(a) + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + 1, c.Length());
    ASSERT_STREQ(TEST_STRING_LONG_A "b", c.Str());
}

TEST(String, Concatenate_MoveString_MoveString_Long1)
{
    String a("a");
    String b(TEST_STRING_LONG_B);
    const String c = std::move(a) + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + 1, c.Length());
    ASSERT_STREQ("a" TEST_STRING_LONG_B, c.Str());
}

TEST(String, Concatenate_MoveString_MoveString_Long2)
{
    String a(TEST_STRING_LONG_A);
    String b(TEST_STRING_LONG_B);
    const String c = std::move(a) + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + TEST_STRING_LONG_LEN, c.Length());
    ASSERT_STREQ(TEST_STRING_LONG_A TEST_STRING_LONG_B, c.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Concatenate_Char_StringView)
{
    const StringView b(TEST_STRING_MID_B);
    const String c = 'a' + b;

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(12u, c.Length());
    ASSERT_STREQ("a" TEST_STRING_MID_B, c.Str());
}

TEST(String, Concatenate_StringView_Char)
{
    const StringView a(TEST_STRING_MID_A);
    const String c = a + 'b';

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(12u, c.Length());
    ASSERT_STREQ(TEST_STRING_MID_A "b", c.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Concatenate_Char_MoveString_Short)
{
    String b("b");
    const String c = 'a' + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_MoveString_Char_Short)
{
    String a("a");
    const String c = std::move(a) + 'b';

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(2u, c.Length());
    ASSERT_STREQ("ab", c.Str());
}

TEST(String, Concatenate_Char_MoveString_Long)
{
    String b(TEST_STRING_LONG_B);
    const String c = 'a' + std::move(b);

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(1u + TEST_STRING_LONG_LEN, c.Length());
    ASSERT_STREQ("a" TEST_STRING_LONG_B, c.Str());
}

TEST(String, Concatenate_MoveString_Char_Long)
{
    String a(TEST_STRING_LONG_A);
    const String c = std::move(a) + 'b';

    ASSERT_FALSE(c.Empty());
    ASSERT_EQ(TEST_STRING_LONG_LEN + 1u, c.Length());
    ASSERT_STREQ(TEST_STRING_LONG_A "b", c.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Insert_Char_Empty)
{
    String a;
    a.Insert(0, 'a');

    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(1u, a.Length());
    ASSERT_STREQ("a", a.Str());
}

TEST(String, Insert_Char_Short)
{
    String a("ABCD");

    a.Insert(0, 'a');
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(5u, a.Length());
    ASSERT_STREQ("aABCD", a.Str());

    a.Insert(5, 'a');
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(6u, a.Length());
    ASSERT_STREQ("aABCDa", a.Str());

    a.Insert(3, 'a');
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(7u, a.Length());
    ASSERT_STREQ("aABaCDa", a.Str());
}

TEST(String, Insert_StringView_Empty)
{
    String a;
    a.Insert(0, StringView("abc"));

    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(3u, a.Length());
    ASSERT_STREQ("abc", a.Str());
}

TEST(String, Insert_StringView)
{
    String a("ABCD");

    a.Insert(0, StringView("abc"));
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(7u, a.Length());
    ASSERT_STREQ("abcABCD", a.Str());

    a.Insert(7, StringView("def"));
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(10u, a.Length());
    ASSERT_STREQ("abcABCDdef", a.Str());

    a.Insert(5, StringView("ghijklm"));
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(17u, a.Length());
    ASSERT_STREQ("abcABghijklmCDdef", a.Str());
}

//////////////////////////////////////////////////////////////////////////

TEST(String, Erase)
{
    String a("abcdefghijklmn");

    a.Erase(0, 0);
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(14u, a.Length());
    ASSERT_STREQ("abcdefghijklmn", a.Str());

    a.Erase(5, 0);
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(14u, a.Length());
    ASSERT_STREQ("abcdefghijklmn", a.Str());

    a.Erase(14, 0);
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(14u, a.Length());
    ASSERT_STREQ("abcdefghijklmn", a.Str());

    a.Erase(0, 1);
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(13u, a.Length());
    ASSERT_STREQ("bcdefghijklmn", a.Str());

    a.Erase(10, 123);
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(10u, a.Length());
    ASSERT_STREQ("bcdefghijk", a.Str());

    a.Erase(4, 2);
    ASSERT_FALSE(a.Empty());
    ASSERT_EQ(8u, a.Length());
    ASSERT_STREQ("bcdehijk", a.Str());

    a.Erase(0, 8);
    ASSERT_TRUE(a.Empty());
    ASSERT_EQ(0u, a.Length());
    ASSERT_STREQ("", a.Str());
}

TEST(String, Replace_Char_NoErase)
{
    {
        String a("abc");
        a.Replace(0, 0, 'A');
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(4u, a.Length());
        ASSERT_STREQ("Aabc", a.Str());
    }

    {
        String a("abc");
        a.Replace(1, 0, 'A');
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(4u, a.Length());
        ASSERT_STREQ("aAbc", a.Str());
    }

    {
        String a("abc");
        a.Replace(3, 0, 'A');
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(4u, a.Length());
        ASSERT_STREQ("abcA", a.Str());
    }
}

TEST(String, Replace_Char)
{
    {
        String a("abc");
        a.Replace(0, 1, 'A');
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(3u, a.Length());
        ASSERT_STREQ("Abc", a.Str());
    }

    {
        String a("abc");
        a.Replace(1, 1, 'A');
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(3u, a.Length());
        ASSERT_STREQ("aAc", a.Str());
    }

    {
        String a("abc");
        a.Replace(2, 1, 'A');
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(3u, a.Length());
        ASSERT_STREQ("abA", a.Str());
    }

    {
        String a("abc");
        a.Replace(0, 3, 'A');
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(1u, a.Length());
        ASSERT_STREQ("A", a.Str());
    }
}

TEST(String, Replace_StringView_NoErase)
{
    {
        String a("abc");
        a.Replace(0, 0, StringView("ABC"));
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(6u, a.Length());
        ASSERT_STREQ("ABCabc", a.Str());
    }

    {
        String a("abc");
        a.Replace(1, 0, StringView("ABC"));
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(6u, a.Length());
        ASSERT_STREQ("aABCbc", a.Str());
    }

    {
        String a("abc");
        a.Replace(3, 0, StringView("ABC"));
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(6u, a.Length());
        ASSERT_STREQ("abcABC", a.Str());
    }
}

TEST(String, Replace_StringView)
{
    {
        String a("abc");
        a.Replace(0, 1, StringView("ABC"));
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(5u, a.Length());
        ASSERT_STREQ("ABCbc", a.Str());
    }

    {
        String a("abc");
        a.Replace(1, 1, StringView("ABC"));
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(5u, a.Length());
        ASSERT_STREQ("aABCc", a.Str());
    }

    {
        String a("abc");
        a.Replace(2, 1, StringView("ABC"));
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(5u, a.Length());
        ASSERT_STREQ("abABC", a.Str());
    }

    {
        String a("abc");
        a.Replace(0, 3, StringView("ABC"));
        ASSERT_FALSE(a.Empty());
        ASSERT_EQ(3u, a.Length());
        ASSERT_STREQ("ABC", a.Str());
    }
}

TEST(String, Compare_String_String)
{
    const String stringA("abcd");
    const String stringB("abcde");

    EXPECT_FALSE(stringA == stringB);
    EXPECT_FALSE(stringB == stringA);
    EXPECT_FALSE(stringA >= stringB);
    EXPECT_TRUE(stringB >= stringA);
    EXPECT_TRUE(stringA <= stringB);
    EXPECT_FALSE(stringB <= stringA);

    EXPECT_TRUE(stringA != stringB);
    EXPECT_TRUE(stringB != stringA);
    EXPECT_TRUE(stringA < stringB);
    EXPECT_FALSE(stringB < stringA);
    EXPECT_FALSE(stringA > stringB);
    EXPECT_TRUE(stringB > stringA);
}

TEST(String, Compare_String_StringView)
{
    const String stringA("abcd");
    const StringView stringB("abcde");

    EXPECT_FALSE(stringA == stringB);
    EXPECT_FALSE(stringB == stringA);
    EXPECT_FALSE(stringA >= stringB);
    EXPECT_TRUE(stringB >= stringA);
    EXPECT_TRUE(stringA <= stringB);
    EXPECT_FALSE(stringB <= stringA);

    EXPECT_TRUE(stringA != stringB);
    EXPECT_TRUE(stringB != stringA);
    EXPECT_TRUE(stringA < stringB);
    EXPECT_FALSE(stringB < stringA);
    EXPECT_FALSE(stringA > stringB);
    EXPECT_TRUE(stringB > stringA);
}

TEST(String, Compare_StringView_String)
{
    const StringView stringA("abcd");
    const String stringB("abcde");

    EXPECT_FALSE(stringA == stringB);
    EXPECT_FALSE(stringB == stringA);
    EXPECT_FALSE(stringA >= stringB);
    EXPECT_TRUE(stringB >= stringA);
    EXPECT_TRUE(stringA <= stringB);
    EXPECT_FALSE(stringB <= stringA);

    EXPECT_TRUE(stringA != stringB);
    EXPECT_TRUE(stringB != stringA);
    EXPECT_TRUE(stringA < stringB);
    EXPECT_FALSE(stringB < stringA);
    EXPECT_FALSE(stringA > stringB);
    EXPECT_TRUE(stringB > stringA);
}