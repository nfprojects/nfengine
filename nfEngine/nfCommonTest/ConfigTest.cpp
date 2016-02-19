/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for config parser.
 */

#include "PCH.hpp"
#include "../nfCommon/Config.hpp"

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
    int = 1
    nestedObject = { bla = 123 bla = 321 }
    arrayInObject = [ 42 ]
}

array = [1 2 3]

emptyObject = { }
emptyArray = [ ]
twoDimensionArray = [ [1 2] [3 4] ]
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
    Config config;

    // integers
    EXPECT_TRUE(config.Parse("a=1"));
    EXPECT_TRUE(config.Parse("a=-1"));
    EXPECT_TRUE(config.Parse("a=+1"));
    EXPECT_TRUE(config.Parse("a=0xFFFF"));

    // booleans
    EXPECT_TRUE(config.Parse("a=true"));
    EXPECT_TRUE(config.Parse("a=false"));

    // floats
    EXPECT_TRUE(config.Parse("a=1.0"));
    EXPECT_TRUE(config.Parse("a=-1.0"));
    EXPECT_TRUE(config.Parse("a=1.0e+1"));
    EXPECT_TRUE(config.Parse("a=1.0e-1"));
    EXPECT_TRUE(config.Parse("a=-1.0e+1"));
    EXPECT_TRUE(config.Parse("a=-1.0e-1"));

    // strings
    EXPECT_TRUE(config.Parse("a=\"string\""));

    // objects
    EXPECT_TRUE(config.Parse("a={}"));
    EXPECT_TRUE(config.Parse("a={b=1}"));
    EXPECT_TRUE(config.Parse("a={b=1 c=2}"));

    // arrays
    EXPECT_TRUE(config.Parse("a=[]"));
    EXPECT_TRUE(config.Parse("a=[1]"));
    EXPECT_TRUE(config.Parse("a=[1 2]"));
    EXPECT_TRUE(config.Parse("a=[{b=2}]"));
}

// simple negative tests
TEST(Config, SimpleNegative)
{
    Config config;

    // missing value
    EXPECT_FALSE(config.Parse("a"));
    EXPECT_FALSE(config.Parse("a="));

    // invalid value
    EXPECT_FALSE(config.Parse("a=f"));


    // invalid key
    EXPECT_FALSE(config.Parse("2=2"));
    EXPECT_FALSE(config.Parse("\"string\"=2"));

    // missing closing bracket
    EXPECT_FALSE(config.Parse("a=["));

    // unexpected character
    EXPECT_FALSE(config.Parse("^a=1"));
    EXPECT_FALSE(config.Parse("a^=1"));
    EXPECT_FALSE(config.Parse("a=^1"));
    EXPECT_FALSE(config.Parse("a=1^"));
    EXPECT_FALSE(config.Parse("a==2"));
    EXPECT_FALSE(config.Parse("{"));
    EXPECT_FALSE(config.Parse("["));
    EXPECT_FALSE(config.Parse("="));
    EXPECT_FALSE(config.Parse("}"));
    EXPECT_FALSE(config.Parse("]"));
    EXPECT_FALSE(config.Parse("0"));
    EXPECT_FALSE(config.Parse("\"test\""));
}

// iterate through a test config and chech values
TEST(Config, ValueCheck)
{
    Config config;
    EXPECT_TRUE(config.Parse(TEST_CONFIG));
    // config.DebugPrint(config.GetRootNode());

    bool intOccurred = false, boolOccurred = false,
         floatOccurred = false, stringOccurred = false;
    bool arrayCheck[3] = { false, false, false };
    bool subObjectInt = false;

    auto subObjectIteratorCallback = [&] (const char* key, const ConfigValue& value)
    {
        if (strcmp(key, "int") == 0)
        {
            EXPECT_TRUE(value.IsInt());
            EXPECT_EQ(1, value.GetInt());
            subObjectInt = true;
        }
        return true;
    };

    auto arrayIteratorCallback = [&] (int index, const ConfigValue& value)
    {
        if (index == 0)
        {
            EXPECT_TRUE(value.IsInt());
            EXPECT_EQ(1, value.GetInt());
            arrayCheck[0] = true;
        }
        else if (index == 1)
        {
            EXPECT_TRUE(value.IsInt());
            EXPECT_EQ(2, value.GetInt());
            arrayCheck[1] = true;
        }
        else if (index == 2)
        {
            EXPECT_TRUE(value.IsInt());
            EXPECT_EQ(3, value.GetInt());
            arrayCheck[2] = true;
        }
        return true;
    };

    auto rootObjectIteratorCallback = [&] (const char* key, const ConfigValue& value)
    {
        if (strcmp(key, "integerValue") == 0)
        {
            EXPECT_TRUE(value.IsInt());
            EXPECT_EQ(-1, value.GetInt());
            intOccurred = true;
        }
        else if (strcmp(key, "booleanValue") == 0)
        {
            EXPECT_TRUE(value.IsBool());
            EXPECT_EQ(true, value.GetBool());
            boolOccurred = true;
        }
        else if (strcmp(key, "floatValue") == 0)
        {
            EXPECT_TRUE(value.IsFloat());
            EXPECT_EQ(10.0f, value.GetFloat());
            floatOccurred = true;
        }
        else if (strcmp(key, "stringValue") == 0)
        {
            EXPECT_TRUE(value.IsString());
            EXPECT_STREQ("this is a string", value.GetString());
            stringOccurred = true;
        }
        else if (strcmp(key, "object") == 0)
        {
            EXPECT_TRUE(value.IsObject());
            config.Iterate(subObjectIteratorCallback, value.GetObj());
        }
        else if (strcmp(key, "array") == 0)
        {
            EXPECT_TRUE(value.IsArray());
            config.IterateArray(arrayIteratorCallback, value.GetArray());
        }
        return true;
    };

    config.Iterate(rootObjectIteratorCallback);

    EXPECT_TRUE(intOccurred);
    EXPECT_TRUE(boolOccurred);
    EXPECT_TRUE(floatOccurred);
    EXPECT_TRUE(stringOccurred);
    EXPECT_TRUE(subObjectInt);
    EXPECT_TRUE(arrayCheck[0]);
    EXPECT_TRUE(arrayCheck[1]);
    EXPECT_TRUE(arrayCheck[2]);
}

// create a test config structure and verifies if generated config string is correct
TEST(Config, Generate)
{
    const char* REFERENCE_STRING =
        "bool=true int=1 float=123 string=\"str\" obj={blah=123} obj2={blah2=234} array=[1 2]";

    Config config;

    ConfigObject root;
    config.AddValue(root, "bool", ConfigValue(true));
    config.AddValue(root, "int", ConfigValue(1));
    config.AddValue(root, "float", ConfigValue(123.0f));
    config.AddValue(root, "string", ConfigValue("str"));

    ConfigObject subObject;
    config.AddValue(subObject, "blah", ConfigValue(123));
    config.AddValue(root, "obj", ConfigValue(subObject));


    ConfigObject subObject2;
    config.AddValue(subObject2, "blah2", ConfigValue(234));
    config.AddValue(root, "obj2", ConfigValue(subObject2));

    ConfigArray array;
    config.AddValue(array, ConfigValue(1));
    config.AddValue(array, ConfigValue(2));
    config.AddValue(root, "array", ConfigValue(array));

    config.SetRoot(root);

    EXPECT_EQ(REFERENCE_STRING, config.ToString(false));
}
