#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Config/Config.hpp"
#include "Engine/Common/Reflection/SerializationContext.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"
#include "Engine/Common/Memory/DynamicBuffer.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


TEST(ReflectionEnumTest, TestEnum_Verify)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("TestEnum", type->GetName());
    EXPECT_EQ(TypeKind::Enumeration, type->GetKind());
    EXPECT_EQ(sizeof(TestEnum), type->GetSize());
    EXPECT_EQ(alignof(TestEnum), type->GetAlignment());

    const auto& options = type->GetOptions();
    ASSERT_EQ(3u, options.Size());

    EXPECT_STREQ("OptionA", options[0].name.Data());
    EXPECT_STREQ("OptionB", options[1].name.Data());
    EXPECT_STREQ("OptionC", options[2].name.Data());

    EXPECT_EQ(0, options[0].value);
    EXPECT_EQ(3, options[1].value);
    EXPECT_EQ(123, options[2].value);
}

TEST(ReflectionEnumTest, TestEnum_FindOptionByValue)
{
    TestEnum value = TestEnum::OptionB;

    const auto* type = GetType<TestEnum>();
    EXPECT_TRUE(type->FindOptionByValue(type->ReadRawValue(&value)) == "OptionB");
}

TEST(ReflectionEnumTest, TestEnum_FindOptionByValue_Invalid)
{
    TestEnum value = TestEnum::UnknownOption;

    const auto* type = GetType<TestEnum>();
    EXPECT_TRUE(type->FindOptionByValue(type->ReadRawValue(&value)).Empty());
}

TEST(ReflectionEnumTest, TestEnum_WriteRawValue)
{
    TestEnum value = TestEnum::OptionA;

    GetType<TestEnum>()->WriteRawValue(&value, (uint64)TestEnum::OptionC);
    EXPECT_EQ(TestEnum::OptionC, value);
}

TEST(ReflectionEnumTest, Deserialize_Valid)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    TestEnum obj = TestEnum::OptionA;
    const ConfigValue value("OptionB");
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(TestEnum::OptionB, obj);
}

TEST(ReflectionEnumTest, Deserialize_InvalidOption)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    TestEnum obj = TestEnum::OptionA;
    const ConfigValue value("NonExistentOption");
    SerializationContext context;
    ASSERT_FALSE(type->Deserialize(&obj, Config(), value, context));
}

TEST(ReflectionEnumTest, Deserialize_InvalidType)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    TestEnum obj = TestEnum::OptionA;
    const ConfigValue value(123);
    SerializationContext context;
    ASSERT_FALSE(type->Deserialize(&obj, Config(), value, context));
}

TEST(ReflectionEnumTest, Serialize_Valid)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    String str;
    TestEnum obj = TestEnum::OptionA;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=\"OptionA\"";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionEnumTest, Serialize_Invalid)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    String str;
    TestEnum obj = TestEnum::UnknownOption;
    ASSERT_FALSE(helper::SerializeObject(type, &obj, str));
}

TEST(ReflectionEnumTest, SerializeBinary_Valid)
{
    const auto* type = GetType<TestEnum>();

    DynamicBuffer buffer;
    SerializationContext context;
    {
        TestEnum obj = TestEnum::OptionC;
        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        TestEnum readObj = TestEnum::OptionA;
        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));
        EXPECT_EQ(TestEnum::OptionC, readObj);
    }
}

TEST(ReflectionEnumTest, SerializeBinary_InvalidOption)
{
    const auto* type = GetType<TestEnum>();

    DynamicBuffer buffer;
    SerializationContext context;
    {
        TestEnum obj = TestEnum::UnknownOption;
        BufferOutputStream stream(buffer);
        EXPECT_FALSE(helper::SerializeObject(type, &obj, stream, context));
    }
}