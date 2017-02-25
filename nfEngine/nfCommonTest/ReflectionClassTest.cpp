#include "PCH.hpp"
#include "nfCommon/Reflection/Reflection.h"


using namespace NFE;
using namespace NFE::RTTI;


//////////////////////////////////////////////////////////////////////////

class TestBaseClass
{
    NFE_DECLARE_CLASS(TestBaseClass)
public:
    int32 mInteger;
    float mFloat;
private:
    bool mPrivateBool;
};

NFE_BEGIN_CLASS(TestBaseClass)
    NFE_CLASS_MEMBER(mInteger)
    NFE_CLASS_MEMBER(mFloat)
    NFE_CLASS_MEMBER(mPrivateBool)
NFE_END_CLASS()

//////////////////////////////////////////////////////////////////////////

class TestChildClass : public TestBaseClass
{
    NFE_DECLARE_CLASS(TestChildClass)
public:
    uint32 mFoo;
};

NFE_BEGIN_CLASS(TestChildClass)
    NFE_CLASS_PARENT(TestBaseClass)
    NFE_CLASS_MEMBER(mFoo)
NFE_END_CLASS()

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionClassTest, BaseClass)
{
    const size_t minSize = sizeof(size_t) + sizeof(int) + sizeof(float) + sizeof(bool);
    const Type* type = NFE_GET_TYPE(TestBaseClass);
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("TestBaseClass", type->GetName());
    EXPECT_LE(minSize, type->GetSize());
    EXPECT_LE(minSize, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());

    // check class members
    const Type::Members& members = type->GetMembers();
    ASSERT_EQ(3, members.size());

    EXPECT_EQ("mInteger", members[0].GetName());
    EXPECT_EQ(NFE_GET_TYPE(int32), members[0].GetType());
    EXPECT_EQ(sizeof(size_t), members[0].GetOffset()); // size_t - vtable offset

    EXPECT_EQ("mFloat", members[1].GetName());
    EXPECT_EQ(NFE_GET_TYPE(float), members[1].GetType());
    EXPECT_EQ(sizeof(size_t) + sizeof(int32), members[1].GetOffset());

    EXPECT_EQ("mPrivateBool", members[2].GetName());
    EXPECT_EQ(NFE_GET_TYPE(bool), members[2].GetType());
    EXPECT_EQ(sizeof(size_t) + sizeof(int32) + sizeof(float), members[2].GetOffset());
}
