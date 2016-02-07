/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for config parser.
 */

#include "PCH.hpp"
#include "../nfCommon/Config.hpp"
#include "../nfCommon/ConfigDataTranslator.hpp"

using namespace NFE::Common;

namespace {

const char* TEST_CONFIG = R"(
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
    // cp.DebugPrint(cp.GetRootNode());

    bool intOccurred = false, boolOccurred = false,
         floatOccurred = false, stringOccurred = false;

    cp.Iterate([&](const char* key, const ConfigValue& value)
    {
       if (strcmp(key, "integerValue") == 0)
       {
           EXPECT_TRUE(value.IsInt());
           EXPECT_EQ(-1, value.Get<int>());
           intOccurred = true;
       }
       else if (strcmp(key, "booleanValue") == 0)
       {
           EXPECT_TRUE(value.IsBool());
           EXPECT_EQ(true, value.Get<bool>());
           boolOccurred = true;
       }
       else if (strcmp(key, "floatValue") == 0)
       {
           EXPECT_TRUE(value.IsFloat());
           EXPECT_EQ(10.0f, value.Get<float>());
           floatOccurred = true;
       }
       else if (strcmp(key, "stringValue") == 0)
       {
           EXPECT_TRUE(value.IsString());
           EXPECT_STREQ("this is a string", value.Get<const char*>());
           stringOccurred = true;
       }

       // TODO: verify arrays and nested objects
    });

    EXPECT_TRUE(intOccurred);
    EXPECT_TRUE(boolOccurred);
    EXPECT_TRUE(floatOccurred);
    EXPECT_TRUE(stringOccurred);
}

TEST(Config, Generate)
{
    ConfigParser cp;

    ConfigObject* root = cp.CreateObject();
    cp.AddToObject(root, "bool", ConfigValue(true));
    cp.AddToObject(root, "int", ConfigValue(1));
    cp.AddToObject(root, "float", ConfigValue(123.0f));
    cp.AddToObject(root, "string", ConfigValue("a string"));
}

TEST(Config, DataTranslator)
{
    const char* TRANSLATOR_TEST_CONFIG = R"(
    integerValue = -1
    booleanValue = true
    floatValue = 10.0
    stringValue = "this is a string"
    )";

    ConfigParser cp;
    ASSERT_TRUE(cp.Parse(TRANSLATOR_TEST_CONFIG));

    struct TestStruct
    {
        int integerValue;
        bool booleanValue;
        float floatValue;
        const char* stringValue;
    };

    auto translator = DataTranslator<TestStruct>()
        .Add("integerValue", &TestStruct::integerValue)
        .Add("booleanValue", &TestStruct::booleanValue)
        .Add("floatValue", &TestStruct::floatValue)
        .Add("stringValue", &TestStruct::stringValue);

    TestStruct object;
    ASSERT_TRUE(cp.TranslateConfigObject(cp.GetRootNode(), translator, object));
    EXPECT_EQ(-1, object.integerValue);
    EXPECT_EQ(true, object.booleanValue);
    EXPECT_EQ(10.0f, object.floatValue);
    EXPECT_STREQ("this is a string", object.stringValue);
}
