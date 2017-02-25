#include "PCH.hpp"
#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Reflection/ReflectionFundamentalTypes.hpp"
#include "nfCommon/Config/Config.hpp"


using namespace NFE;
using namespace NFE::RTTI;


class TestClass
{
    NFE_DECLARE_CLASS(TestClass)

public:
    int32 mInteger;
    float mFloat;
    bool mBool;
};

NFE_DECLARE_TYPE(TestClass);

//////////////////////////////////////////////////////////////////////////

class TestClassWithNestedType
{
    NFE_DECLARE_CLASS(TestClassWithNestedType)

public:
    TestClass mFoo;
    uint32 mBar;
};

NFE_DECLARE_TYPE(TestClassWithNestedType);

//////////////////////////////////////////////////////////////////////////

NFE_BEGIN_DEFINE_CLASS(TestClass)
    NFE_CLASS_MEMBER(mInteger)
    NFE_CLASS_MEMBER(mFloat)
    NFE_CLASS_MEMBER(mBool)
NFE_END_DEFINE_CLASS()

//////////////////////////////////////////////////////////////////////////

NFE_BEGIN_DEFINE_CLASS(TestClassWithNestedType)
    NFE_CLASS_MEMBER(mFoo)
    NFE_CLASS_MEMBER(mBar)
NFE_END_DEFINE_CLASS()

//////////////////////////////////////////////////////////////////////////


namespace {

size_t TEST_CALSS_SIZE = 12; // int32, float, bool
size_t TEST_CALSS_WITH_NESTED_TYPE_SIZE = TEST_CALSS_SIZE + sizeof(uint32);

} // namespace


TEST(ReflectionClassTest, TestClass)
{
    const Type* type = NFE_GET_TYPE(TestClass);
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("TestClass", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(TEST_CALSS_SIZE, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());

    // check class members
    const Type::Members& members = type->GetMembers();
    ASSERT_EQ(3, members.size());

    EXPECT_EQ("mInteger", members[0].GetName());
    EXPECT_EQ(NFE_GET_TYPE(int32), members[0].GetType());
    EXPECT_EQ(0, members[0].GetOffset());

    EXPECT_EQ("mFloat", members[1].GetName());
    EXPECT_EQ(NFE_GET_TYPE(float), members[1].GetType());
    EXPECT_EQ(sizeof(int32), members[1].GetOffset());

    EXPECT_EQ("mBool", members[2].GetName());
    EXPECT_EQ(NFE_GET_TYPE(bool), members[2].GetType());
    EXPECT_EQ(sizeof(int32) + sizeof(float), members[2].GetOffset());
}

TEST(ReflectionClassTest, TestClassWithNestedType)
{
    const Type* type = NFE_GET_TYPE(TestClassWithNestedType);
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("TestClassWithNestedType", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(TEST_CALSS_WITH_NESTED_TYPE_SIZE, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());

    // check class members
    const Type::Members& members = type->GetMembers();
    ASSERT_EQ(2, members.size());

    EXPECT_EQ("mFoo", members[0].GetName());
    EXPECT_EQ(NFE_GET_TYPE(TestClass), members[0].GetType());
    EXPECT_EQ(0, members[0].GetOffset());

    EXPECT_EQ("mBar", members[1].GetName());
    EXPECT_EQ(NFE_GET_TYPE(uint32), members[1].GetType());
    EXPECT_EQ(TEST_CALSS_SIZE, members[1].GetOffset());
}

TEST(ReflectionClassTest, TestClassSerialization)
{
    const Type* type = NFE_GET_TYPE(TestClass);
    ASSERT_NE(nullptr, type);

    TestClass obj;
    obj.mInteger = 42;
    obj.mFloat = 1.234f;
    obj.mBool = true;

    Common::Config config;
    Common::ConfigObject root;
    ASSERT_TRUE(type->Serialize("obj", &obj, config, root));
    config.SetRoot(root);

    const char* REFERENCE_CONFIG_STRING = "obj={mInteger=42 mFloat=1.234 mBool=true}";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, config.ToString(false));
}