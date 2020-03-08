/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for config parser.
 */

#include "PCH.hpp"
#include "Engine/Common/Config/ConfigDataTranslator.hpp"

using namespace NFE;
using namespace NFE::Common;

namespace {

const char* TEST_CONFIG = R"(
// single line comment = { //
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

const char* TRANSLATOR_TEST_CONFIG = R"(
integerValue = -1
booleanValue = true
floatValue = 10.0
stringValue = "this is a string"
intArray = [ 1 2 3 ]
boolArray = [ true false true ]
floatArray = [ 1.0 2.0 3.0 ]
stringArray = [ "aaa" "bbb" ]
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

    auto subObjectIteratorCallback = [&] (StringView key, const ConfigValue& value)
    {
        if (key == "int")
        {
            EXPECT_TRUE(value.Is<int32>());
            EXPECT_EQ(1, value.Get<int32>());
            subObjectInt = true;
        }
        return true;
    };

    auto arrayIteratorCallback = [&] (size_t index, const ConfigValue& value)
    {
        if (index == 0)
        {
            EXPECT_TRUE(value.Is<int32>());
            EXPECT_EQ(1, value.Get<int32>());
            arrayCheck[0] = true;
        }
        else if (index == 1)
        {
            EXPECT_TRUE(value.Is<int32>());
            EXPECT_EQ(2, value.Get<int32>());
            arrayCheck[1] = true;
        }
        else if (index == 2)
        {
            EXPECT_TRUE(value.Is<int32>());
            EXPECT_EQ(3, value.Get<int32>());
            arrayCheck[2] = true;
        }
        return true;
    };

    auto rootObjectIteratorCallback = [&] (StringView key, const ConfigValue& value)
    {
        if (key == "integerValue")
        {
            EXPECT_TRUE(value.Is<int32>());
            EXPECT_EQ(-1, value.Get<int32>());
            intOccurred = true;
        }
        else if (key == "booleanValue")
        {
            EXPECT_TRUE(value.Is<bool>());
            EXPECT_EQ(true, value.Get<bool>());
            boolOccurred = true;
        }
        else if (key == "floatValue")
        {
            EXPECT_TRUE(value.Is<float>());
            EXPECT_EQ(10.0f, value.Get<float>());
            floatOccurred = true;
        }
        else if (key == "stringValue")
        {
            EXPECT_TRUE(value.IsString());
            EXPECT_STREQ("this is a string", value.GetString());
            stringOccurred = true;
        }
        else if (key == "object")
        {
            EXPECT_TRUE(value.IsObject());
            config.Iterate(subObjectIteratorCallback, value.GetObj());
        }
        else if (key == "array")
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
    config
        .AddValue(root, StringView("bool"), ConfigValue(true))
        .AddValue(root, StringView("int"), ConfigValue(1))
        .AddValue(root, StringView("float"), ConfigValue(123.0f))
        .AddValue(root, StringView("string"), ConfigValue("str"));

    ConfigObject subObject;
    config.AddValue(subObject, StringView("blah"), ConfigValue(123));
    config.AddValue(root, StringView("obj"), ConfigValue(subObject));

    ConfigObject subObject2;
    config.AddValue(subObject2, StringView("blah2"), ConfigValue(234));
    config.AddValue(root, StringView("obj2"), ConfigValue(subObject2));

    ConfigArray array;
    config.AddValue(array, ConfigValue(1)).AddValue(array, ConfigValue(2));
    config.AddValue(root, StringView("array"), ConfigValue(array));

    config.SetRoot(root);

    EXPECT_EQ(REFERENCE_STRING, config.ToString(false));
}

// iterate through a test config and check values
TEST(Config, GenericValue)
{
    Config config;
    EXPECT_TRUE(config.Parse(TEST_CONFIG));
    ConfigGenericValue root(&config);

    ASSERT_TRUE(root.HasMember("integerValue"));
    ASSERT_TRUE(root["integerValue"].Is<int32>());
    EXPECT_EQ(-1, root["integerValue"].Get<int32>());

    ASSERT_TRUE(root.HasMember("hexValue"));
    ASSERT_TRUE(root["hexValue"].Is<int32>());
    EXPECT_EQ(0xFF, root["hexValue"].Get<int32>());

    ASSERT_TRUE(root.HasMember("booleanValue"));
    ASSERT_TRUE(root["booleanValue"].Is<bool>());
    EXPECT_EQ(true, root["booleanValue"].Get<bool>());

    ASSERT_TRUE(root.HasMember("floatValue"));
    ASSERT_TRUE(root["floatValue"].Is<float>());
    EXPECT_EQ(10.0f, root["floatValue"].Get<float>());

    ASSERT_TRUE(root.HasMember("stringValue"));
    ASSERT_TRUE(root["stringValue"].IsString());
    EXPECT_STREQ("this is a string", root["stringValue"].GetString());

    ASSERT_TRUE(root.HasMember("object"));
    ASSERT_TRUE(root["object"].IsObject());
    {
        ConfigGenericValue object = root["object"];

        ASSERT_TRUE(object.HasMember("int"));
        ASSERT_TRUE(object["int"].Is<int32>());
        EXPECT_EQ(1, object["int"].Get<int32>());

        ASSERT_TRUE(object.HasMember("nestedObject"));
        ASSERT_TRUE(object["nestedObject"].IsObject());
        ASSERT_TRUE(object["nestedObject"].HasMember("bla"));
        ASSERT_TRUE(object["nestedObject"]["bla"].Is<int32>());
        EXPECT_EQ(123, object["nestedObject"]["bla"].Get<int32>());
    }

    ASSERT_TRUE(root.HasMember("array"));
    ASSERT_TRUE(root["array"].IsArray());
    ASSERT_EQ(3u, root["array"].GetSize());
    EXPECT_EQ(1, root["array"][0u].Get<int32>());
    EXPECT_EQ(2, root["array"][1u].Get<int32>());
    EXPECT_EQ(3, root["array"][2u].Get<int32>());

    ASSERT_TRUE(root.HasMember("emptyObject"));
    ASSERT_TRUE(root["emptyObject"].IsObject());

    ASSERT_TRUE(root.HasMember("emptyArray"));
    ASSERT_TRUE(root["emptyArray"].IsArray());
    EXPECT_EQ(0u, root["emptyArray"].GetSize());

    ASSERT_TRUE(root.HasMember("twoDimensionArray"));
    ASSERT_TRUE(root["twoDimensionArray"].IsArray());

    ASSERT_TRUE(root.HasMember("arrayOfObjects"));
    ASSERT_TRUE(root["arrayOfObjects"].IsArray());
}

TEST(Config, DataTranslator)
{
    Config config;
    ASSERT_TRUE(config.Parse(TRANSLATOR_TEST_CONFIG));

    struct TestStruct
    {
        int integerValue;
        bool booleanValue;
        float floatValue;
        const char* stringValue;

        DynArray<int> intArray;
        DynArray<bool> boolArray;
        DynArray<float> floatArray;
        DynArray<const char*> stringArray;
    };

    // set up data translator
    auto translator = DataTranslator<TestStruct>()
        .Add(StringView("integerValue"), &TestStruct::integerValue)
        .Add(StringView("booleanValue"), &TestStruct::booleanValue)
        .Add(StringView("floatValue"), &TestStruct::floatValue)
        .Add(StringView("stringValue"), &TestStruct::stringValue)
        .Add(StringView("intArray"), &TestStruct::intArray)
        .Add(StringView("boolArray"), &TestStruct::boolArray)
        .Add(StringView("floatArray"), &TestStruct::floatArray)
        .Add(StringView("stringArray"), &TestStruct::stringArray);

    TestStruct object;
    ASSERT_TRUE(config.TranslateConfigObject(config.GetRootNode(), translator, object));

    EXPECT_EQ(-1, object.integerValue);
    EXPECT_EQ(true, object.booleanValue);
    EXPECT_EQ(10.0f, object.floatValue);
    EXPECT_STREQ("this is a string", object.stringValue);

    ASSERT_EQ(3u, object.intArray.Size());
    EXPECT_EQ(1, object.intArray[0]);
    EXPECT_EQ(2, object.intArray[1]);
    EXPECT_EQ(3, object.intArray[2]);

    ASSERT_EQ(3u, object.boolArray.Size());
    EXPECT_TRUE(object.boolArray[0]);
    EXPECT_FALSE(object.boolArray[1]);
    EXPECT_TRUE(object.boolArray[2]);

    ASSERT_EQ(3u, object.floatArray.Size());
    EXPECT_EQ(1.0f, object.floatArray[0]);
    EXPECT_EQ(2.0f, object.floatArray[1]);
    EXPECT_EQ(3.0f, object.floatArray[2]);

    ASSERT_EQ(2u, object.stringArray.Size());
    EXPECT_STREQ("aaa", object.stringArray[0]);
    EXPECT_STREQ("bbb", object.stringArray[1]);
}

TEST(Config, DataTranslatorInvalidTypes)
{
    Config config;
    ASSERT_TRUE(config.Parse(TRANSLATOR_TEST_CONFIG));

    struct TestStruct
    {
        int integerValue;
        bool booleanValue;
        float floatValue;
        const char* stringValue;

        DynArray<int> intArray;
        DynArray<bool> boolArray;
        DynArray<float> floatArray;
        DynArray<const char*> stringArray;
    };

    // set up data translator with messed up types
    auto translator = DataTranslator<TestStruct>()
        .Add(StringView("integerValue"), &TestStruct::stringValue)
        .Add(StringView("booleanValue"), &TestStruct::floatValue)
        .Add(StringView("floatValue"), &TestStruct::booleanValue)
        .Add(StringView("stringValue"), &TestStruct::integerValue)
        .Add(StringView("intArray"), &TestStruct::stringArray)
        .Add(StringView("boolArray"), &TestStruct::floatArray)
        .Add(StringView("floatArray"), &TestStruct::boolArray)
        .Add(StringView("stringArray"), &TestStruct::intArray);

    TestStruct object;
    ASSERT_FALSE(config.TranslateConfigObject(config.GetRootNode(), translator, object));
}
