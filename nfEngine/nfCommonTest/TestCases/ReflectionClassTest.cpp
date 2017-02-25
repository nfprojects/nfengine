#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Reflection/ReflectionFundamentalTypes.hpp"
#include "nfCommon/Reflection/ReflectionDynArray.hpp"
#include "nfCommon/Config/Config.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


//////////////////////////////////////////////////////////////////////////
// Test types declarations
//////////////////////////////////////////////////////////////////////////

class TestClassWithFundamentalMembers
{
    NFE_DECLARE_CLASS(TestClassWithFundamentalMembers)

public:
    int32 intValue;
    float floatValue;
    bool boolValue;

    TestClassWithFundamentalMembers()
        : intValue(0)
        , floatValue(0.0f)
        , boolValue(false)
    {}

    TestClassWithFundamentalMembers(int32 intValue, float floatValue, bool boolValue)
        : intValue(intValue)
        , floatValue(floatValue)
        , boolValue(boolValue)
    {}
};

NFE_DECLARE_TYPE(TestClassWithFundamentalMembers);

//////////////////////////////////////////////////////////////////////////

class TestClassWithNestedType
{
    NFE_DECLARE_CLASS(TestClassWithNestedType)

public:
    TestClassWithFundamentalMembers foo;
    int32 bar;
};

NFE_DECLARE_TYPE(TestClassWithNestedType);

//////////////////////////////////////////////////////////////////////////

class TestClassWithArrayType
{
    NFE_DECLARE_CLASS(TestClassWithArrayType)

public:
    int32 array[5];
    float foo;
};

NFE_DECLARE_TYPE(TestClassWithArrayType);

//////////////////////////////////////////////////////////////////////////

class TestClassWithDynArrayType
{
    NFE_DECLARE_CLASS(TestClassWithDynArrayType)

public:
    DynArray<int32> arrayOfInts;
    DynArray<TestClassWithFundamentalMembers> arrayOfObjects;
};

NFE_DECLARE_TYPE(TestClassWithDynArrayType);

//////////////////////////////////////////////////////////////////////////
// Test types definitions
//////////////////////////////////////////////////////////////////////////

NFE_BEGIN_DEFINE_CLASS(TestClassWithFundamentalMembers)
    NFE_CLASS_MEMBER(intValue)
    NFE_CLASS_MEMBER(floatValue)
    NFE_CLASS_MEMBER(boolValue)
NFE_END_DEFINE_CLASS()

//////////////////////////////////////////////////////////////////////////

NFE_BEGIN_DEFINE_CLASS(TestClassWithNestedType)
    NFE_CLASS_MEMBER(foo)
    NFE_CLASS_MEMBER(bar)
NFE_END_DEFINE_CLASS()

//////////////////////////////////////////////////////////////////////////

NFE_BEGIN_DEFINE_CLASS(TestClassWithArrayType)
    NFE_CLASS_MEMBER(array)
    NFE_CLASS_MEMBER(foo)
NFE_END_DEFINE_CLASS()

//////////////////////////////////////////////////////////////////////////

NFE_BEGIN_DEFINE_CLASS(TestClassWithDynArrayType)
    NFE_CLASS_MEMBER(arrayOfInts)
    NFE_CLASS_MEMBER(arrayOfObjects)
NFE_END_DEFINE_CLASS()


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


namespace {

size_t TEST_CALSS_SIZE = 12; // int32, float, bool
size_t TEST_CALSS_WITH_NESTED_TYPE_SIZE = TEST_CALSS_SIZE + sizeof(int32);
size_t TEST_CALSS_WITH_ARRAY_TYPE_SIZE = sizeof(int32) * 5 + sizeof(float);
size_t TEST_CALSS_WITH_DYNARRAY_TYPE_SIZE = sizeof(DynArray<int32>) + sizeof(DynArray<TestClassWithFundamentalMembers>);

} // namespace


TEST(ReflectionClassTest, TestClassWithFundamentalMembers_Verify)
{
    const Type* type = NFE_GET_TYPE(TestClassWithFundamentalMembers);
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestClassWithFundamentalMembers", type->GetName());
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
        EXPECT_STREQ("intValue", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(int32), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }

    {
        const auto& member = members[1];
        EXPECT_STREQ("floatValue", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(float), member.GetType());
        EXPECT_EQ(sizeof(int32), member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }

    {
        const auto& member = members[2];
        EXPECT_STREQ("boolValue", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(bool), member.GetType());
        EXPECT_EQ(sizeof(int32) + sizeof(float), member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }
}

TEST(ReflectionClassTest, TestClassWithFundamentalMembers_Serialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithFundamentalMembers);
    ASSERT_NE(nullptr, type);

    TestClassWithFundamentalMembers obj;
    {
        obj.intValue = 42;
        obj.floatValue = 1.234f;
        obj.boolValue = true;
    }

    std::string str;
    helper::SerializeObject(type, &obj, str);
    EXPECT_STREQ("obj={intValue=42 floatValue=1.234 boolValue=true}", str.c_str());
}

TEST(ReflectionClassTest, TestClassWithFundamentalMembers_Deserialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithFundamentalMembers);
    ASSERT_NE(nullptr, type);

    TestClassWithFundamentalMembers obj;
    helper::DeserializeObject(type, &obj, "obj={intValue=42 floatValue=1.234 boolValue=true}");

    EXPECT_EQ(42, obj.intValue);
    EXPECT_EQ(1.234f, obj.floatValue);
    EXPECT_EQ(true, obj.boolValue);
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionClassTest, TestClassWithNestedType_Verify)
{
    const Type* type = NFE_GET_TYPE(TestClassWithNestedType);
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestClassWithNestedType", type->GetName());
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
        EXPECT_STREQ("foo", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(TestClassWithFundamentalMembers), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }

    {
        const auto& member = members[1];
        EXPECT_STREQ("bar", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(int32), member.GetType());
        EXPECT_EQ(TEST_CALSS_SIZE, member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }
}

TEST(ReflectionClassTest, TestClassWithNestedType_Serialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithNestedType);
    ASSERT_NE(nullptr, type);

    TestClassWithNestedType obj;
    {
        obj.bar = 123;
        obj.foo.intValue = 789;
        obj.foo.floatValue = 16.0f;
        obj.foo.boolValue = false;
    }

    std::string str;
    helper::SerializeObject(type, &obj, str);
    EXPECT_STREQ("obj={foo={intValue=789 floatValue=16 boolValue=false} bar=123}", str.c_str());
}

TEST(ReflectionClassTest, TestClassWithNestedType_Deserialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithNestedType);
    ASSERT_NE(nullptr, type);

    TestClassWithNestedType obj;
    helper::DeserializeObject(type, &obj, "obj={foo={floatValue=16.0 intValue=789 boolValue=true} bar=123}");

    EXPECT_EQ(123, obj.bar);
    EXPECT_EQ(789, obj.foo.intValue);
    EXPECT_EQ(16.0f, obj.foo.floatValue);
    EXPECT_EQ(true, obj.foo.boolValue);
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionClassTest, TestClassWithArrayType_Verify)
{
    const Type* type = NFE_GET_TYPE(TestClassWithArrayType);
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestClassWithArrayType", type->GetName());
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
        EXPECT_STREQ("array", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(int32), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
        EXPECT_EQ(5, member.GetArraySize());
    }

    {
        const auto& member = members[1];
        EXPECT_STREQ("foo", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(float), member.GetType());
        EXPECT_EQ(5 * sizeof(float), member.GetOffset());
        EXPECT_EQ(0, member.GetArraySize());
    }
}

TEST(ReflectionClassTest, TestClassWithArrayType_Serialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithArrayType);
    ASSERT_NE(nullptr, type);

    TestClassWithArrayType obj;
    {
        obj.array[0] = 10;
        obj.array[1] = 20;
        obj.array[2] = 30;
        obj.array[3] = 40;
        obj.array[4] = 50;
        obj.foo = 123.0;
    }

    std::string str;
    helper::SerializeObject(type, &obj, str);
    EXPECT_STREQ("obj={array=[10 20 30 40 50] foo=123}", str.c_str());
}

TEST(ReflectionClassTest, TestClassWithArrayType_Deserialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithArrayType);
    ASSERT_NE(nullptr, type);

    TestClassWithArrayType obj;
    helper::DeserializeObject(type, &obj, "obj={array=[100 200 300 400 500] foo=321.0}");

    EXPECT_EQ(100, obj.array[0]);
    EXPECT_EQ(200, obj.array[1]);
    EXPECT_EQ(300, obj.array[2]);
    EXPECT_EQ(400, obj.array[3]);
    EXPECT_EQ(500, obj.array[4]);
    EXPECT_EQ(321.0f, obj.foo);
}

TEST(ReflectionClassTest, TestClassWithArrayType_Deserialization_TooShort)
{
    const Type* type = NFE_GET_TYPE(TestClassWithArrayType);
    ASSERT_NE(nullptr, type);

    TestClassWithArrayType obj;
    helper::DeserializeObject(type, &obj, "obj={array=[100 200 300] foo=321.0}");

    EXPECT_EQ(100, obj.array[0]);
    EXPECT_EQ(200, obj.array[1]);
    EXPECT_EQ(300, obj.array[2]);
    // TODO uncomment when default values are implemented
    // EXPECT_EQ(0, obj.array[3]);
    // EXPECT_EQ(0, obj.array[4]);
    EXPECT_EQ(321.0f, obj.foo);
}

TEST(ReflectionClassTest, TestClassWithArrayType_Deserialization_TooLong)
{
    const Type* type = NFE_GET_TYPE(TestClassWithArrayType);
    ASSERT_NE(nullptr, type);

    TestClassWithArrayType obj;
    helper::DeserializeObject(type, &obj, "obj={array=[100 200 300 400 500 600 700] foo=321.0}");

    EXPECT_EQ(100, obj.array[0]);
    EXPECT_EQ(200, obj.array[1]);
    EXPECT_EQ(300, obj.array[2]);
    EXPECT_EQ(400, obj.array[3]);
    EXPECT_EQ(500, obj.array[4]);
    EXPECT_EQ(321.0f, obj.foo);
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionClassTest, TestClassWithDynArrayType_Verify)
{
    const Type* type = NFE_GET_TYPE(TestClassWithDynArrayType);
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestClassWithDynArrayType", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(TEST_CALSS_WITH_DYNARRAY_TYPE_SIZE, type->GetSize());
    EXPECT_EQ(sizeof(size_t), type->GetAlignment()); // alignment of DynArray depends on pointer size
    EXPECT_EQ(nullptr, type->GetParent());

    // check class members
    ASSERT_EQ(2, type->GetNumOfMembers());
    std::vector<Member> members;
    type->ListMembers(members);
    ASSERT_EQ(2, members.size());

    {
        const auto& member = members[0];
        EXPECT_STREQ("arrayOfInts", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(DynArray<int32>), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
    }

    {
        const auto& member = members[1];
        EXPECT_STREQ("arrayOfObjects", member.GetName());
        EXPECT_EQ(NFE_GET_TYPE(DynArray<TestClassWithFundamentalMembers>), member.GetType());
        EXPECT_EQ(sizeof(DynArray<int32>), member.GetOffset());
    }
}

TEST(ReflectionClassTest, TestClassWithDynArrayType_Serialization_Empty)
{
    const Type* type = NFE_GET_TYPE(TestClassWithDynArrayType);
    ASSERT_NE(nullptr, type);

    TestClassWithDynArrayType obj;

    std::string str;
    helper::SerializeObject(type, &obj, str);
    EXPECT_STREQ("obj={arrayOfInts=[] arrayOfObjects=[]}", str.c_str());
}

TEST(ReflectionClassTest, TestClassWithDynArrayType_Serialization)
{
    const Type* type = NFE_GET_TYPE(TestClassWithDynArrayType);
    ASSERT_NE(nullptr, type);

    TestClassWithDynArrayType obj;
    {
        obj.arrayOfInts.PushBack(10);
        obj.arrayOfInts.PushBack(20);
        obj.arrayOfInts.PushBack(30);

        obj.arrayOfObjects.PushBack(TestClassWithFundamentalMembers(1, 1.0f, false));
        obj.arrayOfObjects.PushBack(TestClassWithFundamentalMembers(15, 15.0f, false));
    }

    std::string str;
    helper::SerializeObject(type, &obj, str);
    EXPECT_STREQ("obj={arrayOfInts=[10 20 30 40 50] arrayOfObjects=[]}", str.c_str());
}