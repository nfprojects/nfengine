#include "PCH.hpp"
#include "nfCommon/Reflection/Test/ReflectionTestTypes.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;

TEST(ReflectionClassTest, Test_InternalTestClass)
{
    const auto* type = GetType<InternalTestClass>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("NFE::RTTI::InternalTestClass", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(sizeof(int32), type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());

    // check class members
    ASSERT_EQ(1, type->GetNumOfMembers());
    ClassType::Members members;
    type->ListMembers(members);
    ASSERT_EQ(1, members.Size());

    {
        const auto& member = members[0];
        EXPECT_STREQ("foo", member.GetName());
        EXPECT_EQ(GetType<int32>(), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
    }
}

TEST(ReflectionClassTest, Test_InternalTestClassWithNestedObject)
{
    const auto* type = GetType<InternalTestClassWithNestedObject>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_STREQ("NFE::RTTI::InternalTestClassWithNestedObject", type->GetName());
    EXPECT_EQ(TypeKind::SimpleClass, type->GetKind());
    EXPECT_EQ(sizeof(InternalTestClass), type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());

    // check class members
    ASSERT_EQ(1, type->GetNumOfMembers());
    ClassType::Members members;
    type->ListMembers(members);
    ASSERT_EQ(1, members.Size());

    {
        const auto& member = members[0];
        EXPECT_STREQ("foo", member.GetName());
        EXPECT_EQ(GetType<InternalTestClass>(), member.GetType());
        EXPECT_EQ(0, member.GetOffset());
    }
}
