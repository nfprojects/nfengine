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
floatValue = 10.0f
stringValue = "this is a string"

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

    // whitespaces
    EXPECT_TRUE(cp.Parse(" a = true "));
}

/*
// simple negative tests
TEST(Config, SimpleNegative)
{
    ConfigParser cp;

    // missing value
    EXPECT_FALSE(cp.Parse("a"));
    EXPECT_FALSE(cp.Parse("a="));

    // invalid value
    EXPECT_FALSE(cp.Parse("a=f"));

    // missing closing bracket
    EXPECT_FALSE(cp.Parse("a={"));
    EXPECT_FALSE(cp.Parse("a=["));

    // unexpected character
    EXPECT_FALSE(cp.Parse("^a=1"));
    EXPECT_FALSE(cp.Parse("a^=1"));
    EXPECT_FALSE(cp.Parse("a=^1"));
    EXPECT_FALSE(cp.Parse("a=1^"));
}
*/

TEST(Config, ValueCheck)
{
    ConfigParser cp;

    EXPECT_TRUE(cp.Parse(TEST_CONFIG));
}