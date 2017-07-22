#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "nfCommon/Config/Config.hpp"
#include "nfCommon/Containers/DynArray.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


TEST(ReflectionEnumTest, TestClassWithFundamentalMembers_Verify)
{
    const auto* type = GetType<TestEnum>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestEnum", type->GetName());
    EXPECT_EQ(TypeKind::Enumeration, type->GetKind());
    EXPECT_EQ(sizeof(TestEnum), type->GetSize());
    EXPECT_EQ(alignof(TestEnum), type->GetAlignment());


}

/*
TEST(ReflectionClassTest, Serialization)
{
    const auto* type = GetType<TestClassWithFundamentalMembers>();
    ASSERT_NE(nullptr, type);

    TestClassWithFundamentalMembers obj;
    {
        obj.intValue = 42;
        obj.floatValue = 1.234f;
        obj.boolValue = true;
        obj.strValue = "test";
    }

    std::string str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={intValue=42 floatValue=1.234 boolValue=true strValue=\"test\"}", str.c_str());
}

TEST(ReflectionClassTest, Deserialization)
{
    const auto* type = GetType<TestClassWithFundamentalMembers>();
    ASSERT_NE(nullptr, type);

    TestClassWithFundamentalMembers obj;
    helper::DeserializeObject(type, &obj, "obj={intValue=42 floatValue=1.234 boolValue=true strValue=\"blah\"}");

    EXPECT_EQ(42, obj.intValue);
    EXPECT_EQ(1.234f, obj.floatValue);
    EXPECT_EQ(true, obj.boolValue);
    EXPECT_EQ("blah", obj.strValue);
}
*/