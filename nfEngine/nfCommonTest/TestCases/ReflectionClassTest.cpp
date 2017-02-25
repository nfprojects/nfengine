#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
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

class TestClassWithArrayType
{
    NFE_DECLARE_CLASS(TestClassWithArrayType)

public:
    uint32 mArray[5];
    uint32 mFoo;
};

NFE_DECLARE_TYPE(TestClassWithArrayType);

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

NFE_BEGIN_DEFINE_CLASS(TestClassWithArrayType)
    NFE_CLASS_MEMBER(mArray)
    NFE_CLASS_MEMBER(mFoo)
NFE_END_DEFINE_CLASS()

//////////////////////////////////////////////////////////////////////////

namespace {

size_t TEST_CALSS_SIZE = 12; // int32, float, bool
size_t TEST_CALSS_WITH_NESTED_TYPE_SIZE = TEST_CALSS_SIZE + sizeof(uint32);
size_t TEST_CALSS_WITH_ARRAY_TYPE_SIZE = sizeof(uint32) * 5 + sizeof(uint32);

} // namespace


TEST(ReflectionClassTest, TestClass_Verify)
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
    ASSERT_EQ(3, type->GetNumOfMembers());
    std::vector<Member> members;
    type->ListMembers(members);
    ASSERT_EQ(3, members.size());

    {
        const auto& member = members[0];
        EXPECT_EQ("mInteger", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(int32), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }

    {
        const auto& member = members[1];
        EXPECT_EQ("mFloat", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(float), member.GetType());
        EXPECT_EQ(sizeof(int32), member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }

    {
        const auto& member = members[2];
        EXPECT_EQ("mBool", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(bool), member.GetType());
        EXPECT_EQ(sizeof(int32) + sizeof(float), member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }
}

TEST(ReflectionClassTest, TestClass_Serialization)
{
    const Type* type = NFE_GET_TYPE(TestClass);
    ASSERT_NE(nullptr, type);

    TestClass obj;
    obj.mInteger = 42;
    obj.mFloat = 1.234f;
    obj.mBool = true;

    std::string str;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj={mInteger=42 mFloat=1.234 mBool=true}";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionClassTest, TestClass_Deserialization)
{
    const Type* type = NFE_GET_TYPE(TestClass);
    ASSERT_NE(nullptr, type);

    TestClass obj;
    helper::DeserializeObject(type, &obj, "obj={mInteger=42 mFloat=1.234 mBool=true}");
    EXPECT_EQ(42, obj.mInteger);
    EXPECT_EQ(1.234f, obj.mFloat);
    EXPECT_EQ(true, obj.mBool);
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionClassTest, TestClassWithNestedType_Verify)
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
    ASSERT_EQ(2, type->GetNumOfMembers());
    std::vector<Member> members;
    type->ListMembers(members);
    ASSERT_EQ(2, members.size());

    {
        const auto& member = members[0];
        EXPECT_EQ("mFoo", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(TestClass), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }

    {
        const auto& member = members[1];
        EXPECT_EQ("mBar", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(uint32), member.GetType());
        EXPECT_EQ(TEST_CALSS_SIZE, member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }
}

TEST(ReflectionClassTest, TestClassWithNestedType_Serialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithNestedType);
    ASSERT_NE(nullptr, type);

    TestClassWithNestedType obj;
    obj.mBar = 123;
    obj.mFoo.mInteger = 789;
    obj.mFoo.mFloat = 16.0f;
    obj.mFoo.mBool = false;

    std::string str;
    helper::SerializeObject(type, &obj, str);
    EXPECT_EQ("obj={mFoo={mInteger=789 mFloat=16 mBool=false} mBar=123}", str);
}

TEST(ReflectionClassTest, TestClassWithNestedType_Deserialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithNestedType);
    ASSERT_NE(nullptr, type);

    TestClassWithNestedType obj;
    helper::DeserializeObject(type, &obj, "obj={mFoo={mFloat=16.0 mInteger=789 mBool=true} mBar=123}");
    EXPECT_EQ(123, obj.mBar);
    EXPECT_EQ(789, obj.mFoo.mInteger);
    EXPECT_EQ(16.0f, obj.mFoo.mFloat);
    EXPECT_EQ(true, obj.mFoo.mBool);
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionClassTest, TestClassWithArrayType_Verify)
{
    const Type* type = NFE_GET_TYPE(TestClassWithArrayType);
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("TestClassWithArrayType", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(TEST_CALSS_WITH_ARRAY_TYPE_SIZE, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());

    // check class members
    ASSERT_EQ(2, type->GetNumOfMembers());
    std::vector<Member> members;
    type->ListMembers(members);
    ASSERT_EQ(2, members.size());

    {
        const auto& member = members[0];
        EXPECT_EQ("mArray", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(uint32), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
        EXPECT_EQ(5, member.GetArraySize());
    }

    {
        const auto& member = members[1];
        EXPECT_EQ("mFoo", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(uint32), member.GetType());
        EXPECT_EQ(5 * sizeof(uint32), member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }
}

TEST(ReflectionClassTest, TestClassWithArrayType_Serialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithArrayType);
    ASSERT_NE(nullptr, type);

    TestClassWithArrayType obj;
    obj.mArray[0] = 10;
    obj.mArray[1] = 20;
    obj.mArray[2] = 30;
    obj.mArray[3] = 40;
    obj.mArray[4] = 50;
    obj.mFoo = 123;

    std::string str;
    helper::SerializeObject(type, &obj, str);
    EXPECT_EQ("obj={mArray=[10 20 30 40 50] mFoo=123}", str);
}
