#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "nfCommon/Config/Config.hpp"
#include "nfCommon/Config/ConfigValue.hpp"
#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Reflection/ReflectionFundamentalTypes.hpp"


using namespace NFE;
using namespace NFE::RTTI;


TEST(ReflectionFundamentalTypeTest, VerifyBool)
{
    const Type* type = NFE_GET_TYPE(bool);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("bool", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(1, type->GetSize());
    EXPECT_EQ(1, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt8)
{
    const Type* type = NFE_GET_TYPE(int8);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::int8", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(1, type->GetSize());
    EXPECT_EQ(1, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt16)
{
    const Type* type = NFE_GET_TYPE(int16);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::int16", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(2, type->GetSize());
    EXPECT_EQ(2, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt32)
{
    const Type* type = NFE_GET_TYPE(int32);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::int32", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(4, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt64)
{
    const Type* type = NFE_GET_TYPE(int64);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::int64", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(8, type->GetSize());
    EXPECT_EQ(8, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}


TEST(ReflectionFundamentalTypeTest, VerifyUInt8)
{
    const Type* type = NFE_GET_TYPE(uint8);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::uint8", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(1, type->GetSize());
    EXPECT_EQ(1, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt16)
{
    const Type* type = NFE_GET_TYPE(uint16);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::uint16", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(2, type->GetSize());
    EXPECT_EQ(2, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt32)
{
    const Type* type = NFE_GET_TYPE(uint32);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::uint32", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(4, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt64)
{
    const Type* type = NFE_GET_TYPE(uint64);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::uint64", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(8, type->GetSize());
    EXPECT_EQ(8, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}


TEST(ReflectionFundamentalTypeTest, VerifyFloat)
{
    const Type* type = NFE_GET_TYPE(float);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("float", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(4, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

TEST(ReflectionFundamentalTypeTest, VerifyDouble)
{
    const Type* type = NFE_GET_TYPE(double);

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("double", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(8, type->GetSize());
    EXPECT_EQ(8, type->GetAlignment());
    EXPECT_EQ(nullptr, type->GetParent());
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionFundamentalTypeTest, SerializeBool_False)
{
    const Type* type = NFE_GET_TYPE(bool);

    std::string str;
    bool obj = false;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=false";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeBool_True)
{
    const Type* type = NFE_GET_TYPE(bool);

    std::string str;
    bool obj = true;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=true";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt8)
{
    const Type* type = NFE_GET_TYPE(int8);

    std::string str;
    int8 obj = -53;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=-53";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt16)
{
    const Type* type = NFE_GET_TYPE(int16);

    std::string str;
    int16 obj = -1415;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=-1415";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt32)
{
    const Type* type = NFE_GET_TYPE(int32);

    std::string str;
    int32 obj = -1415365;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=-1415365";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt64)
{
    const Type* type = NFE_GET_TYPE(int64);

    std::string str;
    int64 obj = -14152740275365;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=-14152740275365";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt8)
{
    const Type* type = NFE_GET_TYPE(uint8);

    std::string str;
    uint8 obj = 123u;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=123";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt16)
{
    const Type* type = NFE_GET_TYPE(uint16);

    std::string str;
    uint16 obj = 62572u;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=62572";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt32)
{
    const Type* type = NFE_GET_TYPE(uint32);

    std::string str;
    uint32 obj = 3837613155u;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=3837613155";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt64)
{
    const Type* type = NFE_GET_TYPE(uint64);

    std::string str;
    uint64 obj = 25236526261571ull;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=25236526261571";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}


TEST(ReflectionFundamentalTypeTest, SerializeFloat)
{
    const Type* type = NFE_GET_TYPE(float);

    std::string str;
    float obj = 123.0f;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=123";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeDouble)
{
    const Type* type = NFE_GET_TYPE(double);

    std::string str;
    double obj = 123.0;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=123";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionFundamentalTypeTest, DeserializeBool_False)
{
    const Type* type = NFE_GET_TYPE(bool);

    bool obj;
    const Common::ConfigValue value(false);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(false, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeBool_True)
{
    const Type* type = NFE_GET_TYPE(bool);

    bool obj;
    const Common::ConfigValue value(true);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(true, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt8)
{
    const Type* type = NFE_GET_TYPE(int8);

    int8 obj;
    const Common::ConfigValue value(static_cast<int8>(-35));
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(-35, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt16)
{
    const Type* type = NFE_GET_TYPE(int16);

    int16 obj;
    const Common::ConfigValue value(static_cast<int16>(-3255));
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(-3255, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt32)
{
    const Type* type = NFE_GET_TYPE(int32);

    int32 obj;
    const Common::ConfigValue value(-3253625);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(-3253625, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt64)
{
    const Type* type = NFE_GET_TYPE(int64);

    int64 obj;
    const Common::ConfigValue value(-10184017501);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(-10184017501, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt8)
{
    const Type* type = NFE_GET_TYPE(uint8);

    uint8 obj;
    const Common::ConfigValue value(static_cast<uint8>(35u));
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(35u, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt16)
{
    const Type* type = NFE_GET_TYPE(uint16);

    uint16 obj;
    const Common::ConfigValue value(static_cast<uint16>(3255u));
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(3255u, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt32)
{
    const Type* type = NFE_GET_TYPE(uint32);

    uint32 obj;
    const Common::ConfigValue value(3253625u);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(3253625u, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt64)
{
    const Type* type = NFE_GET_TYPE(uint64);

    uint64 obj;
    const Common::ConfigValue value(10184017501ull);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(10184017501ull, obj);
}


TEST(ReflectionFundamentalTypeTest, DeserializeFloat)
{
    const Type* type = NFE_GET_TYPE(float);

    float obj;
    const Common::ConfigValue value(1234.0f);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(1234.0f, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeDouble)
{
    const Type* type = NFE_GET_TYPE(double);

    double obj;
    const Common::ConfigValue value(1234.0);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(1234.0, obj);
}