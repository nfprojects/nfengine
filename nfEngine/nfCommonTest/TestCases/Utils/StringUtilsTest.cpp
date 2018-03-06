/**
 * @file
 * @brief  Unit tests for string utils.
 */

#include "PCH.hpp"
#include "nfCommon/Utils/StringUtils.hpp"


using namespace NFE;
using namespace NFE::Common;

TEST(StringUtils, ToStringBool)
{
    EXPECT_EQ(String("true"), ToString(true));
    EXPECT_EQ(String("false"), ToString(false));
}

TEST(StringUtils, ToStringUint32)
{
    EXPECT_EQ(String("0"), ToString(0u));
    EXPECT_EQ(String("1"), ToString(1u));
    EXPECT_EQ(String("9"), ToString(9u));
    EXPECT_EQ(String("10"), ToString(10u));
    EXPECT_EQ(String("11"), ToString(11u));
    EXPECT_EQ(String("99"), ToString(99u));
    EXPECT_EQ(String("101"), ToString(101u));
    EXPECT_EQ(String("4294967295"), ToString(std::numeric_limits<uint32>::max()));
}

TEST(StringUtils, ToStringUint64)
{
    EXPECT_EQ(String("0"), ToString(uint64(0ull)));
    EXPECT_EQ(String("1"), ToString(uint64(1ull)));
    EXPECT_EQ(String("9"), ToString(uint64(9ull)));
    EXPECT_EQ(String("10"), ToString(uint64(10ull)));
    EXPECT_EQ(String("11"), ToString(uint64(11ull)));
    EXPECT_EQ(String("99"), ToString(uint64(99ull)));
    EXPECT_EQ(String("100"), ToString(uint64(100ull)));
    EXPECT_EQ(String("101"), ToString(uint64(101ull)));
    EXPECT_EQ(String("18446744073709551615"), ToString(std::numeric_limits<uint64>::max()));
}

TEST(StringUtils, ToStringInt32)
{
    EXPECT_EQ(String("-1"), ToString(-1));
    EXPECT_EQ(String("-9"), ToString(-9));
    EXPECT_EQ(String("-10"), ToString(-10));
    EXPECT_EQ(String("-11"), ToString(-11));
    EXPECT_EQ(String("-99"), ToString(-99));
    EXPECT_EQ(String("-100"), ToString(-100));
    EXPECT_EQ(String("-101"), ToString(-101));
    EXPECT_EQ(String("-2147483648"), ToString(std::numeric_limits<int32>::min()));
    EXPECT_EQ(String("0"), ToString(0));
    EXPECT_EQ(String("1"), ToString(1));
    EXPECT_EQ(String("9"), ToString(9));
    EXPECT_EQ(String("10"), ToString(10));
    EXPECT_EQ(String("11"), ToString(11));
    EXPECT_EQ(String("99"), ToString(99));
    EXPECT_EQ(String("100"), ToString(100));
    EXPECT_EQ(String("101"), ToString(101));
    EXPECT_EQ(String("2147483647"), ToString(std::numeric_limits<int32>::max()));
}

TEST(StringUtils, ToStringInt64)
{
    EXPECT_EQ(String("-1"), ToString(int64(-1ll)));
    EXPECT_EQ(String("-9"), ToString(int64(-9ll)));
    EXPECT_EQ(String("-10"), ToString(int64(-10ll)));
    EXPECT_EQ(String("-11"), ToString(int64(-11ll)));
    EXPECT_EQ(String("-99"), ToString(int64(-99ll)));
    EXPECT_EQ(String("-100"), ToString(int64(-100ll)));
    EXPECT_EQ(String("-101"), ToString(int64(-101ll)));
    EXPECT_EQ(String("-9223372036854775808"), ToString(std::numeric_limits<int64>::min()));
    EXPECT_EQ(String("0"), ToString(int64(0ll)));
    EXPECT_EQ(String("1"), ToString(int64(1ll)));
    EXPECT_EQ(String("9"), ToString(int64(9ll)));
    EXPECT_EQ(String("10"), ToString(int64(10ll)));
    EXPECT_EQ(String("11"), ToString(int64(11ll)));
    EXPECT_EQ(String("99"), ToString(int64(99ll)));
    EXPECT_EQ(String("100"), ToString(int64(100ll)));
    EXPECT_EQ(String("101"), ToString(int64(101ll)));
    EXPECT_EQ(String("9223372036854775807"), ToString(std::numeric_limits<int64>::max()));
}

TEST(StringUtils, ToStringFloat)
{
    EXPECT_EQ("0", ToString(0.0f));
    EXPECT_EQ("-0", ToString(-0.0f));
    EXPECT_EQ("1", ToString(1.0f));
    EXPECT_EQ("2", ToString(2.0f));
    EXPECT_EQ("-1", ToString(-1.0f));
    EXPECT_EQ("-2", ToString(-2.0f));
    EXPECT_EQ("0.25", ToString(0.25f));
    EXPECT_EQ("0.1", ToString(0.1f));
    EXPECT_EQ("1e+06", ToString(1000000.0f));
}

TEST(StringUtils, ToStringDouble)
{
    EXPECT_EQ("0", ToString(0.0));
    EXPECT_EQ("-0", ToString(-0.0));
    EXPECT_EQ("1", ToString(1.0));
    EXPECT_EQ("2", ToString(2.0));
    EXPECT_EQ("-1", ToString(-1.0));
    EXPECT_EQ("-2", ToString(-2.0));
    EXPECT_EQ("0.25", ToString(0.25));
    EXPECT_EQ("0.1", ToString(0.1));
    EXPECT_EQ("1e+06", ToString(1000000.0));
}

TEST(StringUtils, Split_StringView)
{
    // splitting with default delimiter (newline)
    {
        const StringView a("01\n345\n\n89abcd");
        const StringView b1("01");
        const StringView b2("345");
        const StringView b3("89abcd");

        DynArray<StringView> tokens = Split(a);
        ASSERT_EQ(tokens.Size(), 3u);
        EXPECT_EQ(b1, tokens[0]);
        EXPECT_EQ(b2, tokens[1]);
        EXPECT_EQ(b3, tokens[2]);
    }

    // custom delimiter (space)
    {
        const StringView a("0123 56 89abcdef");
        const StringView b1("0123");
        const StringView b2("56");
        const StringView b3("89abcdef");

        DynArray<StringView> tokens = Split(a, ' ');
        ASSERT_EQ(tokens.Size(), 3u);
        EXPECT_EQ(b1, tokens[0]);
        EXPECT_EQ(b2, tokens[1]);
        EXPECT_EQ(b3, tokens[2]);
    }

    // delimiter at the beginning
    {
        const StringView a("\n01234");
        const StringView b("01234");

        DynArray<StringView> tokens = Split(a);
        ASSERT_EQ(tokens.Size(), 1u);
        EXPECT_EQ(b, tokens[0]);
    }

    // delimiter at the end
    {
        const StringView a("01234\n");
        const StringView b("01234");

        DynArray<StringView> tokens = Split(a);
        ASSERT_EQ(tokens.Size(), 1u);
        EXPECT_EQ(b, tokens[0]);
    }

    // multiple delimiters
    {
        const StringView a("012\n\n\n\n34");
        const StringView b1("012");
        const StringView b2("34");

        DynArray<StringView> tokens = Split(a);
        ASSERT_EQ(tokens.Size(), 2u);
        EXPECT_EQ(b1, tokens[0]);
        EXPECT_EQ(b2, tokens[1]);
    }

    // delimiters only
    {
        const StringView a("\n\n\n\n\n\n");

        DynArray<StringView> tokens = Split(a);
        ASSERT_EQ(tokens.Size(), 0u);
    }

    // multiple delimiters front and back
    {
        const StringView a("\n\n\n0123\n\n\n");
        const StringView b("0123");

        DynArray<StringView> tokens = Split(a);
        ASSERT_EQ(tokens.Size(), 1u);
        EXPECT_EQ(b, tokens[0]);
    }
}

