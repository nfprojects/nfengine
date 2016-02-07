/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for config parser.
 */

#include "PCH.hpp"
#include "../nfCommon/Config.hpp"

using namespace NFE::Common;

namespace {

char* TEST_CONFIG = R"(
// signle line comment = { //
// /*

integerValue = -1
hexValue = 0xFF
booleanValue = true
floatValue = 10.0
stringValue = "this is a string"

object =
{
    a = 1
    b = 3
    nestedObject = { bla = 123 bla = 321 }
    arrayInObject = [ 42 ]
}

emptyObject = { }

array = [1 2 3]
emptyArray = [ ]
twoDimensionArray = [[1 2] [3 4]]
arrayOfObjects = [ { a = 1} { b = 1} ]

stringWithCommentInIt = "blah//"
stringWithCommentInIt2 = "blah/*"

/* multiline comment
* / this is not the end of the comment yet
*/
)";


} // namespace


// simple sanity tests
TEST(Config, Simple)
{
    ConfigParser cp;

    // integers
    EXPECT_TRUE(cp.Parse("a=1"));
    EXPECT_TRUE(cp.Parse("a=-1"));
    EXPECT_TRUE(cp.Parse("a=+1"));

    // booleans
    EXPECT_TRUE(cp.Parse("a=true"));
    EXPECT_TRUE(cp.Parse("a=false"));

    // floats
    EXPECT_TRUE(cp.Parse("a=1.0"));
    EXPECT_TRUE(cp.Parse("a=-1.0"));
    EXPECT_TRUE(cp.Parse("a=1.0e+1"));
    EXPECT_TRUE(cp.Parse("a=1.0e-1"));
    EXPECT_TRUE(cp.Parse("a=-1.0e+1"));
    EXPECT_TRUE(cp.Parse("a=-1.0e-1"));

    // strings
    EXPECT_TRUE(cp.Parse("a=\"string\""));

    // objects
    EXPECT_TRUE(cp.Parse("a={b=1}"));
}

// simple negative tests
TEST(Config, SimpleNegative)
{
    ConfigParser cp;

    // missing value
    EXPECT_FALSE(cp.Parse("a"));
    EXPECT_FALSE(cp.Parse("a="));

    // invalid value
    EXPECT_FALSE(cp.Parse("a=f"));

    // invalid key
    EXPECT_FALSE(cp.Parse("2=2"));
    EXPECT_FALSE(cp.Parse("\"string\"=2"));

    // missing closing bracket
    EXPECT_FALSE(cp.Parse("a=["));

    // unexpected character
    EXPECT_FALSE(cp.Parse("^a=1"));
    EXPECT_FALSE(cp.Parse("a^=1"));
    EXPECT_FALSE(cp.Parse("a=^1"));
    EXPECT_FALSE(cp.Parse("a=1^"));
    EXPECT_FALSE(cp.Parse("a==2"));
}


TEST(Config, ValueCheck)
{
    ConfigParser cp;

    EXPECT_TRUE(cp.Parse(TEST_CONFIG));
    cp.DebugPrint(cp.GetRootNode());
}

/*
TEST(Config, DataTranslator)
{
    ConfigParser cp;

    struct TestStruct
    {
        int integerValue;
        int hexValue;
        bool booleanValue;
        float floatValue;
        char* stringValue;
    };

    EXPECT_TRUE(cp.Parse(TEST_CONFIG));
}
*/