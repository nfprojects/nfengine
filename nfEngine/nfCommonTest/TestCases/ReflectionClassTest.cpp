#include "PCH.hpp"
#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Reflection/ReflectionFundamentalTypes.hpp"


using namespace NFE;
using namespace NFE::RTTI;


class TestClass
{
    NFE_DECLARE_CLASS(TestClass)

public:
    int32 mInteger;
    float mFloat;
private:
    bool mPrivateBool;
};

NFE_DECLARE_TYPE(TestClass);

//////////////////////////////////////////////////////////////////////////

NFE_BEGIN_DEFINE_CLASS(TestClass)
    NFE_CLASS_MEMBER(mInteger)
    NFE_CLASS_MEMBER(mFloat)
    NFE_CLASS_MEMBER(mPrivateBool)
NFE_END_DEFINE_CLASS()

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionClassTest, TestClass)
{
    const size_t minSize = sizeof(size_t) + sizeof(int32) + sizeof(float) + sizeof(bool);
    const Type* type = NFE_GET_TYPE(TestClass);
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("TestClass", type->GetName());
    EXPECT_LE(minSize, type->GetSize());
    EXPECT_LE(sizeof(size_t), type->GetAlignment());
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

    EXPECT_EQ("mPrivateBool", members[2].GetName());
    EXPECT_EQ(NFE_GET_TYPE(bool), members[2].GetType());
    EXPECT_EQ(sizeof(int32) + sizeof(float), members[2].GetOffset());
}