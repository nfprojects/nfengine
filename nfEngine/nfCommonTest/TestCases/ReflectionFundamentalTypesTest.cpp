#include "PCH.hpp"
#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Reflection/ReflectionFundamentalTypes.hpp"

using namespace NFE;
using namespace NFE::RTTI;


TEST(ReflectionFundamentalTypeTest, VerifyBool)
{
    const Type* type = NFE_GET_TYPE(bool);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("bool", type->GetName());
    EXPECT_EQ(1, type->GetSize());
    EXPECT_EQ(1, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}


TEST(ReflectionFundamentalTypeTest, VerifyInt8)
{
    const Type* type = NFE_GET_TYPE(int8);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::int8", type->GetName());
    EXPECT_EQ(1, type->GetSize());
    EXPECT_EQ(1, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt16)
{
    const Type* type = NFE_GET_TYPE(int16);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::int16", type->GetName());
    EXPECT_EQ(2, type->GetSize());
    EXPECT_EQ(2, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt32)
{
    const Type* type = NFE_GET_TYPE(int32);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::int32", type->GetName());
    EXPECT_EQ(4, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt64)
{
    const Type* type = NFE_GET_TYPE(int64);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::int64", type->GetName());
    EXPECT_EQ(8, type->GetSize());
    EXPECT_EQ(8, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}


TEST(ReflectionFundamentalTypeTest, VerifyUInt8)
{
    const Type* type = NFE_GET_TYPE(uint8);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::uint8", type->GetName());
    EXPECT_EQ(1, type->GetSize());
    EXPECT_EQ(1, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt16)
{
    const Type* type = NFE_GET_TYPE(uint16);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::uint16", type->GetName());
    EXPECT_EQ(2, type->GetSize());
    EXPECT_EQ(2, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt32)
{
    const Type* type = NFE_GET_TYPE(uint32);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::uint32", type->GetName());
    EXPECT_EQ(4, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt64)
{
    const Type* type = NFE_GET_TYPE(uint64);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::uint64", type->GetName());
    EXPECT_EQ(8, type->GetSize());
    EXPECT_EQ(8, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}


TEST(ReflectionFundamentalTypeTest, VerifyFloat)
{
    const Type* type = NFE_GET_TYPE(float);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("float", type->GetName());
    EXPECT_EQ(4, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyDouble)
{
    const Type* type = NFE_GET_TYPE(double);
    ASSERT_NE(nullptr, type);
    EXPECT_EQ("double", type->GetName());
    EXPECT_EQ(8, type->GetSize());
    EXPECT_EQ(8, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}
