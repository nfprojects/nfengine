#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


TEST(ReflectionEnumTest, TestEnum_Verify)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestEnum", type->GetName());
    EXPECT_EQ(TypeKind::Enumeration, type->GetKind());
    EXPECT_EQ(sizeof(TestEnum), type->GetSize());
    EXPECT_EQ(alignof(TestEnum), type->GetAlignment());

    const auto& options = type->GetOptions();
    ASSERT_EQ(3, options.Size());

    EXPECT_STREQ("OptionA", options[0].name);
    EXPECT_STREQ("OptionB", options[1].name);
    EXPECT_STREQ("OptionC", options[2].name);

    EXPECT_EQ(0, options[0].value);
    EXPECT_EQ(3, options[1].value);
    EXPECT_EQ(123, options[2].value);
}

TEST(ReflectionEnumTest, Deserialize_Valid)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    TestEnum obj = TestEnum::OptionA;
    const ConfigValue value("OptionB");
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value));
    EXPECT_EQ(TestEnum::OptionB, obj);
}

TEST(ReflectionEnumTest, Deserialize_Invalid)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    TestEnum obj = TestEnum::OptionA;
    const ConfigValue value("NonExistentOption");
    ASSERT_FALSE(type->Deserialize(&obj, Config(), value));
}

TEST(ReflectionEnumTest, Serialize_Valid)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    std::string str;
    TestEnum obj = TestEnum::OptionA;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=\"OptionA\"";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionEnumTest, Serialize_Invalid)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    std::string str;
    TestEnum obj = static_cast<TestEnum>(99); // non-mapped enum value
    ASSERT_FALSE(helper::SerializeObject(type, &obj, str));
}