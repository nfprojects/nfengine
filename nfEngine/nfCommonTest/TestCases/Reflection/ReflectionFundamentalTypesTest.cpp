#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "nfCommon/Config/Config.hpp"
#include "nfCommon/Config/ConfigValue.hpp"
#include "nfCommon/Reflection/ReflectionMacros.hpp"
#include "nfCommon/Reflection/Types/ReflectionFundamentalType.hpp"


using namespace NFE;
using namespace NFE::RTTI;


TEST(ReflectionFundamentalTypeTest, VerifyBool)
{
    const auto* type = GetType<bool>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("bool", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(1, type->GetSize());
    EXPECT_EQ(1, type->GetAlignment());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt8)
{
    const auto* type = GetType<int8>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::int8", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(1, type->GetSize());
    EXPECT_EQ(1, type->GetAlignment());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt16)
{
    const auto* type = GetType<int16>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::int16", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(2, type->GetSize());
    EXPECT_EQ(2, type->GetAlignment());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt32)
{
    const auto* type = GetType<int32>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::int32", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(4, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
}

TEST(ReflectionFundamentalTypeTest, VerifyInt64)
{
    const auto* type = GetType<int64>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::int64", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(8, type->GetSize());
    EXPECT_EQ(8, type->GetAlignment());
}


TEST(ReflectionFundamentalTypeTest, VerifyUInt8)
{
    const auto* type = GetType<uint8>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::uint8", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(1, type->GetSize());
    EXPECT_EQ(1, type->GetAlignment());
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt16)
{
    const auto* type = GetType<uint16>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::uint16", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(2, type->GetSize());
    EXPECT_EQ(2, type->GetAlignment());
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt32)
{
    const auto* type = GetType<uint32>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::uint32", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(4, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt64)
{
    const auto* type = GetType<uint64>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("NFE::uint64", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(8, type->GetSize());
    EXPECT_EQ(8, type->GetAlignment());
}


TEST(ReflectionFundamentalTypeTest, VerifyFloat)
{
    const auto* type = GetType<float>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("float", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(4, type->GetSize());
    EXPECT_EQ(4, type->GetAlignment());
}

TEST(ReflectionFundamentalTypeTest, VerifyDouble)
{
    const auto* type = GetType<double>();

    ASSERT_NE(nullptr, type);
    EXPECT_STREQ("double", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(8, type->GetSize());
    EXPECT_EQ(8, type->GetAlignment());
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionFundamentalTypeTest, SerializeBool_False)
{
    const auto* type = GetType<bool>();

    std::string str;
    bool obj = false;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=false";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeBool_True)
{
    const auto* type = GetType<bool>();

    std::string str;
    bool obj = true;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=true";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt8)
{
    const auto* type = GetType<int8>();

    std::string str;
    int8 obj = -53;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=-53";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt16)
{
    const auto* type = GetType<int16>();

    std::string str;
    int16 obj = -1415;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=-1415";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt32)
{
    const auto* type = GetType<int32>();

    std::string str;
    int32 obj = -1415365;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=-1415365";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt64)
{
    const auto* type = GetType<int64>();

    std::string str;
    int64 obj = -14152740275365;
    helper::SerializeObject(type, &obj, str);
    const char* REFERENCE_CONFIG_STRING = "obj=-14152740275365";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt8)
{
    const auto* type = GetType<uint8>();

    std::string str;
    uint8 obj = 123u;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=123";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt16)
{
    const auto* type = GetType<uint16>();

    std::string str;
    uint16 obj = 62572u;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=62572";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt32)
{
    const auto* type = GetType<uint32>();

    std::string str;
    uint32 obj = 3837613155u;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=3837613155";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt64)
{
    const auto* type = GetType<uint64>();

    std::string str;
    uint64 obj = 25236526261571ull;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=25236526261571";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}


TEST(ReflectionFundamentalTypeTest, SerializeFloat)
{
    const auto* type = GetType<float>();

    std::string str;
    float obj = 123.0f;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=123";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeDouble)
{
    const auto* type = GetType<double>();

    std::string str;
    double obj = 123.0;
    helper::SerializeObject(type, &obj, str);

    const char* REFERENCE_CONFIG_STRING = "obj=123";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionFundamentalTypeTest, DeserializeBool_False)
{
    const auto* type = GetType<bool>();

    bool obj;
    const Common::ConfigValue value(false);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(false, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeBool_True)
{
    const auto* type = GetType<bool>();

    bool obj;
    const Common::ConfigValue value(true);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(true, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt8)
{
    const auto* type = GetType<int8>();

    int8 obj;
    const Common::ConfigValue value(static_cast<int8>(-35));
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(-35, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt16)
{
    const auto* type = GetType<int16>();

    int16 obj;
    const Common::ConfigValue value(static_cast<int16>(-3255));
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(-3255, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt32)
{
    const auto* type = GetType<int32>();

    int32 obj;
    const Common::ConfigValue value(-3253625);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(-3253625, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt64)
{
    const auto* type = GetType<int64>();

    int64 obj;
    const Common::ConfigValue value(-10184017501);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(-10184017501, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt8)
{
    const auto* type = GetType<uint8>();

    uint8 obj;
    const Common::ConfigValue value(static_cast<uint8>(35u));
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(35u, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt16)
{
    const auto* type = GetType<uint16>();

    uint16 obj;
    const Common::ConfigValue value(static_cast<uint16>(3255u));
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(3255u, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt32)
{
    const auto* type = GetType<uint32>();

    uint32 obj;
    const Common::ConfigValue value(3253625u);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(3253625u, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt64)
{
    const auto* type = GetType<uint64>();

    uint64 obj;
    const Common::ConfigValue value(10184017501ull);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(10184017501ull, obj);
}


TEST(ReflectionFundamentalTypeTest, DeserializeFloat)
{
    const auto* type = GetType<float>();

    float obj;
    const Common::ConfigValue value(1234.0f);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(1234.0f, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeDouble)
{
    const auto* type = GetType<double>();

    double obj;
    const Common::ConfigValue value(1234.0);
    ASSERT_TRUE(type->Deserialize(&obj, Common::Config(), value));
    EXPECT_EQ(1234.0, obj);
}