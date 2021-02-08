#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Reflection/ReflectionUtils.hpp"


using namespace NFE;
using namespace NFE::RTTI;
using namespace NFE::Common;


TEST(ReflectionClassTest, BaseClass_Verify)
{
    const size_t minSize = sizeof(size_t) + sizeof(int32) + sizeof(float) + sizeof(bool);
    const auto* type = GetType<TestBaseClass>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("TestBaseClass", type->GetName());
    EXPECT_EQ(TypeKind::Class, type->GetKind());
    EXPECT_FALSE(type->IsAbstract());
    EXPECT_LE(minSize, type->GetSize());
    EXPECT_LE(sizeof(size_t), type->GetAlignment());
    EXPECT_EQ(GetType<IObject>(), type->GetParent());

    // vtable offset
    const size_t initialOffset = sizeof(size_t);

    // check class members
    ASSERT_EQ(3u, type->GetNumOfMembers());
    ClassType::Members members;
    type->ListMembers(members);
    ASSERT_EQ(3u, members.Size());
    {
        EXPECT_STREQ("intVal", members[0].GetName());
        EXPECT_EQ(GetType<int32>(), members[0].GetType());
        EXPECT_EQ(initialOffset + 0, members[0].GetOffset());

        EXPECT_STREQ("floatVal", members[1].GetName());
        EXPECT_EQ(GetType<float>(), members[1].GetType());
        EXPECT_EQ(initialOffset + 4, members[1].GetOffset());

        EXPECT_STREQ("mPrivateBool", members[2].GetName());
        EXPECT_EQ(GetType<bool>(), members[2].GetType());
        EXPECT_EQ(initialOffset + 8, members[2].GetOffset());
    }

    EXPECT_STREQ("intVal", members[0].GetName());
    EXPECT_EQ(GetType<int32>(), members[0].GetType());
    EXPECT_EQ(initialOffset, members[0].GetOffset()); // size_t = vtable offset

    EXPECT_STREQ("floatVal", members[1].GetName());
    EXPECT_EQ(GetType<float>(), members[1].GetType());
    EXPECT_EQ(initialOffset + sizeof(int32), members[1].GetOffset());

    EXPECT_STREQ("mPrivateBool", members[2].GetName());
    EXPECT_EQ(GetType<bool>(), members[2].GetType());
    EXPECT_EQ(initialOffset + sizeof(int32) + sizeof(float), members[2].GetOffset());
}

TEST(ReflectionClassTest, BaseClass_Serialization)
{
    const auto* type = GetType<TestBaseClass>();
    ASSERT_NE(nullptr, type);

    TestBaseClass obj;
    {
        obj.intVal = 123;
        obj.floatVal = 1.1f;
    }

    String str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={intVal=123 floatVal=1.1}", str.Str());
}

TEST(ReflectionClassTest, BaseClass_Deserialization)
{
    const auto* type = GetType<TestBaseClass>();
    ASSERT_NE(nullptr, type);

    TestBaseClass obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj={intVal=42 floatVal=4.321 mPrivateBool=true}"));

    EXPECT_EQ(42, obj.intVal);
    EXPECT_EQ(4.321f, obj.floatVal);
    EXPECT_EQ(true, obj.GetBool());
}


//////////////////////////////////////////////////////////////////////////


TEST(ReflectionClassTest, ChildClass_Verify)
{
    const size_t minSize = sizeof(size_t) + sizeof(int32) + sizeof(float) + sizeof(bool) + sizeof(uint32);
    const auto* type = GetType<TestChildClassA>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("TestChildClassA", type->GetName());
    EXPECT_EQ(TypeKind::Class, type->GetKind());
    EXPECT_FALSE(type->IsAbstract());
    EXPECT_LE(minSize, type->GetSize());
    EXPECT_LE(sizeof(size_t), type->GetAlignment());
    EXPECT_EQ(GetType<TestBaseClass>(), type->GetParent());

    // vtable offset
    const size_t initialOffset = sizeof(size_t);

    // check class members
    ASSERT_EQ(4u, type->GetNumOfMembers());
    ClassType::Members members;
    type->ListMembers(members);
    ASSERT_EQ(4u, members.Size());
    {
        EXPECT_STREQ("intVal", members[0].GetName());
        EXPECT_EQ(GetType<int32>(), members[0].GetType());
        EXPECT_EQ(initialOffset + 0, members[0].GetOffset());

        EXPECT_STREQ("floatVal", members[1].GetName());
        EXPECT_EQ(GetType<float>(), members[1].GetType());
        EXPECT_EQ(initialOffset + 4, members[1].GetOffset());

        EXPECT_STREQ("mPrivateBool", members[2].GetName());
        EXPECT_EQ(GetType<bool>(), members[2].GetType());
        EXPECT_EQ(initialOffset + 8, members[2].GetOffset());

        EXPECT_STREQ("foo", members[3].GetName());
        EXPECT_EQ(GetType<int32>(), members[3].GetType());
        EXPECT_GE(members[3].GetOffset(), members[2].GetOffset() + alignof(int32)); // so it's compatible with both VS and GCC
    }
}

TEST(ReflectionClassTest, ChildClass_Cast_Down)
{
    TestChildClassA objA;
    EXPECT_EQ(static_cast<const TestBaseClass*>(&objA), Cast<TestBaseClass>(&objA));
}

TEST(ReflectionClassTest, ChildClass_Cast_Up)
{
    TestChildClassA objA;
    TestBaseClass* basePtr = &objA;

    EXPECT_EQ(&objA, Cast<TestChildClassA>(basePtr));
    EXPECT_EQ(nullptr, Cast<TestChildClassB>(basePtr));
}

TEST(ReflectionClassTest, ChildClass_Serialization)
{
    const auto* type = GetType<TestChildClassA>();
    ASSERT_NE(nullptr, type);

    TestChildClassA obj;
    {
        obj.intVal = 456;
        obj.floatVal = 3.14f;
        obj.foo = 12345;
    }

    String str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={intVal=456 floatVal=3.14 foo=12345}", str.Str());
}

TEST(ReflectionClassTest, ChildClass_Deserialization)
{
    const auto* type = GetType<TestChildClassA>();
    ASSERT_NE(nullptr, type);

    TestChildClassA obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj={intVal=999 floatVal=2.71 mPrivateBool=true foo=76543}"));

    EXPECT_EQ(999, obj.intVal);
    EXPECT_EQ(2.71f, obj.floatVal);
    EXPECT_EQ(true, obj.GetBool());
    EXPECT_EQ(76543, obj.foo);
}

TEST(ReflectionClassTest, ChildClass_Deserialization_InvalidMarker)
{
    const auto* type = GetType<TestChildClassA>();
    ASSERT_NE(nullptr, type);

    TestChildClassA obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj,
        "obj={__type=\"InvalidTypeName\" intVal=999 floatVal=2.71 mPrivateBool=true foo=76543}"));
}


//////////////////////////////////////////////////////////////////////////


TEST(ReflectionClassTest, AbstractClass_Verify)
{
    const size_t minSize = sizeof(size_t) + sizeof(int32) + sizeof(float);
    const auto* type = GetType<TestAbstractClass>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("TestAbstractClass", type->GetName());
    EXPECT_EQ(TypeKind::Class, type->GetKind());
    EXPECT_TRUE(type->IsAbstract());
    EXPECT_LE(minSize, type->GetSize());
    EXPECT_LE(sizeof(size_t), type->GetAlignment());
    EXPECT_EQ(GetType<IObject>(), type->GetParent());

    // vtable offset
    const size_t initialOffset = sizeof(size_t);

    // check class members
    ASSERT_EQ(2u, type->GetNumOfMembers());
    ClassType::Members members;
    type->ListMembers(members);
    ASSERT_EQ(2u, members.Size());
    {
        EXPECT_STREQ("intVal", members[0].GetName());
        EXPECT_EQ(GetType<int32>(), members[0].GetType());
        EXPECT_EQ(initialOffset + 0, members[0].GetOffset());

        EXPECT_STREQ("floatVal", members[1].GetName());
        EXPECT_EQ(GetType<float>(), members[1].GetType());
        EXPECT_EQ(initialOffset + 4, members[1].GetOffset());
    }
}

TEST(ReflectionClassTest, AbstractClass_Serialization)
{
    const auto* type = GetType<TestAbstractClass>();
    ASSERT_NE(nullptr, type);

    TestChildClassA obj;
    {
        obj.intVal = 456;
        obj.floatVal = 3.14f;
        obj.foo = 12345;
    }

    String str;
    ASSERT_FALSE(helper::SerializeObject(type, &obj, str));
}

TEST(ReflectionClassTest, AbstractClass_Deserialization)
{
    const auto* type = GetType<TestAbstractClass>();
    ASSERT_NE(nullptr, type);

    TestChildClassA obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj, "obj={intVal=999 floatVal=2.71 mPrivateBool=true foo=76543}"));
}


//////////////////////////////////////////////////////////////////////////

TEST(ReflectionClassTest, DynamicCast_FromChild)
{
    UniquePtr<TestChildClassA> pointer = MakeUniquePtr<TestChildClassA>();

    const TestBaseClass* pointerBase = RTTI::Cast<TestBaseClass>(pointer.Get());
    EXPECT_EQ(pointerBase, pointer.Get());

    const TestChildClassA* pointerA = RTTI::Cast<TestChildClassA>(pointer.Get());
    EXPECT_EQ(pointerA, pointer.Get());
}

TEST(ReflectionClassTest, DynamicCast_FromBase)
{
    const UniquePtr<TestBaseClass> pointer = MakeUniquePtr<TestChildClassA>();

    const TestBaseClass* pointerBase = RTTI::Cast<TestBaseClass>(pointer.Get());
    EXPECT_EQ(pointerBase, pointer.Get());

    const TestChildClassA* pointerA = RTTI::Cast<TestChildClassA>(pointer.Get());
    EXPECT_EQ(pointerA, pointer.Get());

    const TestChildClassB* pointerB = RTTI::Cast<TestChildClassB>(pointer.Get());
    EXPECT_EQ(pointerB, nullptr);
}

TEST(ReflectionClassTest, DynamicCast_FromChild_Nullptr)
{
    const TestChildClassA* pointer = nullptr;

    const TestBaseClass* pointerBase = RTTI::Cast<TestBaseClass>(pointer);
    EXPECT_EQ(pointerBase, nullptr);
}

TEST(ReflectionClassTest, DynamicCast_FromBase_Nullptr)
{
    const TestBaseClass* pointer = nullptr;

    const TestBaseClass* pointerBase = RTTI::Cast<TestBaseClass>(pointer);
    EXPECT_EQ(pointerBase, nullptr);
}
