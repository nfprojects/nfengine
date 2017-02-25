#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "nfCommon/Config/Config.hpp"
#include "nfCommon/Containers/DynArray.hpp"
#include "nfCommon/Reflection/ReflectionMacros.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


TEST(ReflectionClassTest, TestClassWithFundamentalMembers_Verify)
{
    const auto* type = GetType<TestClassWithFundamentalMembers>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestClassWithFundamentalMembers", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(sizeof(TestClassWithFundamentalMembers), type->GetSize());
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
        EXPECT_EQ(GetType<int32>(), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
    }

    {
        const auto& member = members[1];
        EXPECT_STREQ("floatValue", member.GetName());
        EXPECT_EQ(GetType<float>(), member.GetType());
        EXPECT_EQ(sizeof(int32), member.GetOffset());
    }

    {
        const auto& member = members[2];
        EXPECT_STREQ("boolValue", member.GetName());
        EXPECT_EQ(GetType<bool>(), member.GetType());
        EXPECT_EQ(sizeof(int32) + sizeof(float), member.GetOffset());
    }
}

TEST(ReflectionClassTest, TestClassWithFundamentalMembers_Serialization)
{
    const auto* type = GetType<TestClassWithFundamentalMembers>();
    ASSERT_NE(nullptr, type);

    TestClassWithFundamentalMembers obj;
    {
        obj.intValue = 42;
        obj.floatValue = 1.234f;
        obj.boolValue = true;
    }

    std::string str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={intValue=42 floatValue=1.234 boolValue=true}", str.c_str());
}

TEST(ReflectionClassTest, TestClassWithFundamentalMembers_Deserialization)
{
    const auto* type = GetType<TestClassWithFundamentalMembers>();
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
    const auto* type = GetType<TestClassWithNestedType>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestClassWithNestedType", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(sizeof(TestClassWithNestedType), type->GetSize());
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
        EXPECT_EQ(GetType<TestClassWithFundamentalMembers>(), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
    }

    {
        const auto& member = members[1];
        EXPECT_STREQ("bar", member.GetName());
        EXPECT_EQ(GetType<int32>(), member.GetType());
        EXPECT_EQ(sizeof(TestClassWithFundamentalMembers), member.GetOffset());
    }
}

TEST(ReflectionClassTest, TestClassWithNestedType_Serialization)
{
    const auto* type = GetType<TestClassWithNestedType>();
    ASSERT_NE(nullptr, type);

    TestClassWithNestedType obj;
    {
        obj.bar = 123;
        obj.foo.intValue = 789;
        obj.foo.floatValue = 16.0f;
        obj.foo.boolValue = false;
    }

    std::string str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={foo={intValue=789 floatValue=16 boolValue=false} bar=123}", str.c_str());
}

TEST(ReflectionClassTest, TestClassWithNestedType_Deserialization)
{
    const auto* type = GetType<TestClassWithNestedType>();
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
    const auto* type = GetType<TestClassWithArrayType>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestClassWithArrayType", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(sizeof(TestClassWithArrayType), type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());

    // check class members
    ASSERT_EQ(2, type->GetNumOfMembers());
    std::vector<Member> members;
    type->ListMembers(members);
    ASSERT_EQ(2, members.size());

    {
        const auto& member = members[0];
        EXPECT_STREQ("arrayOfInts", member.GetName());
        EXPECT_EQ(GetType<int32[5]>(), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
    }

    {
        const auto& member = members[1];
        EXPECT_STREQ("foo", member.GetName());
        EXPECT_EQ(GetType<float>(), member.GetType());
        EXPECT_EQ(5 * sizeof(float), member.GetOffset());
    }
}

TEST(ReflectionClassTest, TestClassWithArrayType_Serialization)
{
    const Type* type = GetType<TestClassWithArrayType>();
    ASSERT_NE(nullptr, type);

    TestClassWithArrayType obj;
    {
        obj.arrayOfInts[0] = 10;
        obj.arrayOfInts[1] = 20;
        obj.arrayOfInts[2] = 30;
        obj.arrayOfInts[3] = 40;
        obj.arrayOfInts[4] = 50;
        obj.foo = 123.0;
    }

    std::string str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={arrayOfInts=[10 20 30 40 50] foo=123}", str.c_str());
}

TEST(ReflectionClassTest, TestClassWithArrayType_Deserialization)
{
    const Type* type = GetType<TestClassWithArrayType>();
    ASSERT_NE(nullptr, type);

    TestClassWithArrayType obj;
    helper::DeserializeObject(type, &obj, "obj={arrayOfInts=[100 200 300 400 500] foo=321.0}");

    EXPECT_EQ(100, obj.arrayOfInts[0]);
    EXPECT_EQ(200, obj.arrayOfInts[1]);
    EXPECT_EQ(300, obj.arrayOfInts[2]);
    EXPECT_EQ(400, obj.arrayOfInts[3]);
    EXPECT_EQ(500, obj.arrayOfInts[4]);
    EXPECT_EQ(321.0f, obj.foo);
}

TEST(ReflectionClassTest, TestClassWithArrayType_Deserialization_TooShort)
{
    const Type* type = GetType<TestClassWithArrayType>();
    ASSERT_NE(nullptr, type);

    TestClassWithArrayType obj;
    helper::DeserializeObject(type, &obj, "obj={arrayOfInts=[100 200 300] foo=321.0}");

    EXPECT_EQ(100, obj.arrayOfInts[0]);
    EXPECT_EQ(200, obj.arrayOfInts[1]);
    EXPECT_EQ(300, obj.arrayOfInts[2]);
    // TODO uncomment when default values are implemented
    // EXPECT_EQ(0, obj.array[3]);
    // EXPECT_EQ(0, obj.array[4]);
    EXPECT_EQ(321.0f, obj.foo);
}

TEST(ReflectionClassTest, TestClassWithArrayType_Deserialization_TooLong)
{
    const Type* type = GetType<TestClassWithArrayType>();
    ASSERT_NE(nullptr, type);

    TestClassWithArrayType obj;
    helper::DeserializeObject(type, &obj, "obj={arrayOfInts=[100 200 300 400 500 600 700] foo=321.0}");

    EXPECT_EQ(100, obj.arrayOfInts[0]);
    EXPECT_EQ(200, obj.arrayOfInts[1]);
    EXPECT_EQ(300, obj.arrayOfInts[2]);
    EXPECT_EQ(400, obj.arrayOfInts[3]);
    EXPECT_EQ(500, obj.arrayOfInts[4]);
    EXPECT_EQ(321.0f, obj.foo);
}


//////////////////////////////////////////////////////////////////////////


TEST(ReflectionClassTest, TestClassWithDynArrayType_Verify)
{
    const auto* type = GetType<TestClassWithDynArrayType>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestClassWithDynArrayType", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(sizeof(TestClassWithDynArrayType), type->GetSize());
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
        EXPECT_EQ(GetType<DynArray<int32>>(), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
    }

    {
        const auto& member = members[1];
        EXPECT_STREQ("arrayOfObjects", member.GetName());
        EXPECT_EQ(GetType<DynArray<TestClassWithFundamentalMembers>>(), member.GetType());
        EXPECT_EQ(sizeof(DynArray<int32>), member.GetOffset());
    }
}

TEST(ReflectionClassTest, TestClassWithDynArrayType_Serialization_Empty)
{
    const auto* type = GetType<TestClassWithDynArrayType>();
    ASSERT_NE(nullptr, type);

    TestClassWithDynArrayType obj;

    std::string str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={arrayOfInts=[] arrayOfObjects=[]}", str.c_str());
}

TEST(ReflectionClassTest, TestClassWithDynArrayType_Serialization)
{
    const auto* type = GetType<TestClassWithDynArrayType>();
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
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={arrayOfInts=[10 20 30] arrayOfObjects=[{intValue=1 floatValue=1 boolValue=false} {intValue=15 floatValue=15 boolValue=false}]}", str.c_str());
}

TEST(ReflectionClassTest, TestClassWithArrayType_Deserialization_Empty)
{
    const auto* type = GetType<TestClassWithDynArrayType>();
    ASSERT_NE(nullptr, type);

    TestClassWithDynArrayType obj;
    helper::DeserializeObject(type, &obj, "obj={arrayOfInts=[] arrayOfObjects=[]}");

    EXPECT_EQ(0, obj.arrayOfInts.Size());
    EXPECT_EQ(0, obj.arrayOfObjects.Size());
}

TEST(ReflectionClassTest, TestClassWithArrayType_Deserialization_NonEmpty)
{
    const auto* type = GetType<TestClassWithDynArrayType>();
    ASSERT_NE(nullptr, type);

    TestClassWithDynArrayType obj;
    helper::DeserializeObject(type, &obj, "obj={arrayOfInts=[2 3 5 7] arrayOfObjects=[{intValue=321 floatValue=10.0 boolValue=true}]}");

    ASSERT_EQ(4, obj.arrayOfInts.Size());
    EXPECT_EQ(2, obj.arrayOfInts[0]);
    EXPECT_EQ(3, obj.arrayOfInts[1]);
    EXPECT_EQ(5, obj.arrayOfInts[2]);
    EXPECT_EQ(7, obj.arrayOfInts[3]);

    ASSERT_EQ(1, obj.arrayOfObjects.Size());
    EXPECT_EQ(321, obj.arrayOfObjects[0].intValue);
    EXPECT_EQ(10.0f, obj.arrayOfObjects[0].floatValue);
    EXPECT_EQ(true, obj.arrayOfObjects[0].boolValue);
}

//////////////////////////////////////////////////////////////////////////


TEST(ReflectionClassTest, TestClassWithUniquePtrType_Verify)
{
    const auto* type = GetType<TestClassWithUniquePtrType>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("TestClassWithUniquePtrType", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(sizeof(TestClassWithUniquePtrType), type->GetSize());
    EXPECT_EQ(sizeof(size_t), type->GetAlignment()); // alignment of DynArray depends on pointer size
    EXPECT_EQ(nullptr, type->GetParent());

    // check class members
    ASSERT_EQ(4, type->GetNumOfMembers());
    std::vector<Member> members;
    type->ListMembers(members);
    ASSERT_EQ(4, members.size());

    size_t totalOffset = 0;

    {
        const auto& member = members[0];
        EXPECT_STREQ("pointerToInt", member.GetName());
        EXPECT_EQ(GetType<std::unique_ptr<NFE::int32>>(), member.GetType());
        EXPECT_EQ(totalOffset, member.GetOffset());
        totalOffset += sizeof(std::unique_ptr<NFE::int32>);
    }

    {
        const auto& member = members[1];
        EXPECT_STREQ("pointerToObject", member.GetName());
        EXPECT_EQ(GetType<std::unique_ptr<TestClassWithFundamentalMembers>>(), member.GetType());
        EXPECT_EQ(totalOffset, member.GetOffset());
        totalOffset += sizeof(std::unique_ptr<TestClassWithFundamentalMembers>);
    }

    {
        const auto& member = members[2];
        EXPECT_STREQ("pointerToArrayOfInts", member.GetName());
        EXPECT_EQ(GetType<std::unique_ptr<NFE::Common::DynArray<NFE::int32>>>(), member.GetType());
        EXPECT_EQ(totalOffset, member.GetOffset());
        totalOffset += sizeof(std::unique_ptr<NFE::Common::DynArray<NFE::int32>>);
    }

    {
        const auto& member = members[3];
        EXPECT_STREQ("arrayOfPointersToInts", member.GetName());
        EXPECT_EQ(GetType<NFE::Common::DynArray<std::unique_ptr<NFE::int32>>>(), member.GetType());
        EXPECT_EQ(totalOffset, member.GetOffset());
        totalOffset += sizeof(NFE::Common::DynArray<std::unique_ptr<NFE::int32>>);
    }
}

TEST(ReflectionClassTest, TestClassWithUniquePtrType_Serialization_Empty)
{
    const auto* type = GetType<TestClassWithUniquePtrType>();
    ASSERT_NE(nullptr, type);

    TestClassWithUniquePtrType obj;
    obj.arrayOfPointersToInts.PushBack(nullptr);

    std::string str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={pointerToInt=0 pointerToObject=0 pointerToArrayOfInts=0 arrayOfPointersToInts=[0]}", str.c_str());
}