#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Config/Config.hpp"
#include "Engine/Common/Reflection/SerializationContext.hpp"
#include "Engine/Common/Reflection/ReflectionUtils.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"
#include "Engine/Common/Memory/Buffer.hpp"


using namespace NFE;
using namespace NFE::RTTI;
using namespace NFE::Common;


TEST(ReflectionFundamentalTypeTest, VerifyBool)
{
    const auto* type = GetType<bool>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("bool", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(1u, type->GetSize());
    EXPECT_EQ(1u, type->GetAlignment());

    const bool* defaultObject = GetDefaultObject<bool>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(false, *defaultObject);
}

TEST(ReflectionFundamentalTypeTest, VerifyInt8)
{
    const auto* type = GetType<int8>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::int8", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(1u, type->GetSize());
    EXPECT_EQ(1u, type->GetAlignment());

    const int8* defaultObject = GetDefaultObject<int8>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0, *defaultObject);
}

TEST(ReflectionFundamentalTypeTest, VerifyInt16)
{
    const auto* type = GetType<int16>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::int16", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(2u, type->GetSize());
    EXPECT_EQ(2u, type->GetAlignment());

    const int16* defaultObject = GetDefaultObject<int16>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0, *defaultObject);
}

TEST(ReflectionFundamentalTypeTest, VerifyInt32)
{
    const auto* type = GetType<int32>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::int32", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(4u, type->GetSize());
    EXPECT_EQ(4u, type->GetAlignment());

    const int32* defaultObject = GetDefaultObject<int32>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0, *defaultObject);
}

TEST(ReflectionFundamentalTypeTest, VerifyInt64)
{
    const auto* type = GetType<int64>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::int64", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(8u, type->GetSize());
    EXPECT_EQ(8u, type->GetAlignment());

    const int64* defaultObject = GetDefaultObject<int64>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0, *defaultObject);
}


TEST(ReflectionFundamentalTypeTest, VerifyUInt8)
{
    const auto* type = GetType<uint8>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::uint8", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(1u, type->GetSize());
    EXPECT_EQ(1u, type->GetAlignment());

    const uint8* defaultObject = GetDefaultObject<uint8>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0u, *defaultObject);
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt16)
{
    const auto* type = GetType<uint16>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::uint16", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(2u, type->GetSize());
    EXPECT_EQ(2u, type->GetAlignment());

    const uint16* defaultObject = GetDefaultObject<uint16>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0u, *defaultObject);
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt32)
{
    const auto* type = GetType<uint32>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::uint32", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(4u, type->GetSize());
    EXPECT_EQ(4u, type->GetAlignment());

    const uint32* defaultObject = GetDefaultObject<uint32>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0u, *defaultObject);
}

TEST(ReflectionFundamentalTypeTest, VerifyUInt64)
{
    const auto* type = GetType<uint64>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("NFE::uint64", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(8u, type->GetSize());
    EXPECT_EQ(8u, type->GetAlignment());

    const uint64* defaultObject = GetDefaultObject<uint64>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0u, *defaultObject);
}


TEST(ReflectionFundamentalTypeTest, VerifyFloat)
{
    const auto* type = GetType<float>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("float", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(4u, type->GetSize());
    EXPECT_EQ(4u, type->GetAlignment());

    const float* defaultObject = GetDefaultObject<float>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0.0f, *defaultObject);
}

TEST(ReflectionFundamentalTypeTest, VerifyDouble)
{
    const auto* type = GetType<double>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("double", type->GetName());
    EXPECT_EQ(TypeKind::Fundamental, type->GetKind());
    EXPECT_EQ(8u, type->GetSize());
    EXPECT_EQ(8u, type->GetAlignment());

    const double* defaultObject = GetDefaultObject<double>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(0.0, *defaultObject);
}

TEST(ReflectionFundamentalTypeTest, VerifyString)
{
    const auto* type = GetType<String>();

    ASSERT_NE(nullptr, type);
    EXPECT_EQ("String", type->GetName());
    EXPECT_EQ(TypeKind::String, type->GetKind());
    EXPECT_EQ(sizeof(NFE::Common::String), type->GetSize());
    EXPECT_EQ(alignof(NFE::Common::String), type->GetAlignment());

    const String* defaultObject = GetDefaultObject<String>();
    ASSERT_NE(nullptr, defaultObject);
    EXPECT_EQ(String(), *defaultObject);
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionFundamentalTypeTest, SerializeBool_False)
{
    const auto* type = GetType<bool>();

    String str;
    bool obj = false;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=false";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeBool_True)
{
    const auto* type = GetType<bool>();

    String str;
    bool obj = true;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    const char* REFERENCE_CONFIG_STRING = "obj=true";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeBinaryBool)
{
    const auto* type = GetType<bool>();

    Buffer buffer;
    SerializationContext context;
    {
        bool obj = false;
        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
        EXPECT_EQ(1u, buffer.Size());
    }
    {
        bool readObj = true;
        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));
        EXPECT_EQ(false, readObj);
    }
}

TEST(ReflectionFundamentalTypeTest, SerializeInt8)
{
    const auto* type = GetType<int8>();

    String str;
    int8 obj = -53;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    const char* REFERENCE_CONFIG_STRING = "obj=-53";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt16)
{
    const auto* type = GetType<int16>();

    String str;
    int16 obj = -1415;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    const char* REFERENCE_CONFIG_STRING = "obj=-1415";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeInt32)
{
    const auto* type = GetType<int32>();

    String str;
    int32 obj = -1415365;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    const char* REFERENCE_CONFIG_STRING = "obj=-1415365";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeBinaryInt32)
{
    const auto* type = GetType<int32>();

    Buffer buffer;
    SerializationContext context;
    {
        int32 obj = -1415365;
        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
        EXPECT_EQ(4u, buffer.Size());
    }
    {
        int32 readObj = 19246194;
        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));
        EXPECT_EQ(-1415365, readObj);
    }
}

TEST(ReflectionFundamentalTypeTest, SerializeInt64)
{
    const auto* type = GetType<int64>();

    String str;
    int64 obj = -14152740275365ll;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    const char* REFERENCE_CONFIG_STRING = "obj=-14152740275365";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt8)
{
    const auto* type = GetType<uint8>();

    String str;
    uint8 obj = 123u;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=123";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt16)
{
    const auto* type = GetType<uint16>();

    String str;
    uint16 obj = 62572u;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=62572";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt32)
{
    const auto* type = GetType<uint32>();

    String str;
    uint32 obj = 3837613155u;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=3837613155";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeUInt64)
{
    const auto* type = GetType<uint64>();

    String str;
    uint64 obj = 25236526261571ull;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=25236526261571";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}


TEST(ReflectionFundamentalTypeTest, SerializeFloat)
{
    const auto* type = GetType<float>();

    String str;
    float obj = 123.0f;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=123";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeDouble)
{
    const auto* type = GetType<double>();

    String str;
    double obj = 123.0;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=123";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeString_Empty)
{
    const auto* type = GetType<String>();

    String str;
    const String obj;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=\"\"";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

TEST(ReflectionFundamentalTypeTest, SerializeString_NonEmpty)
{
    const auto* type = GetType<String>();

    String str;
    const String obj("qwertyuiop12345");
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));

    const char* REFERENCE_CONFIG_STRING = "obj=\"qwertyuiop12345\"";
    EXPECT_EQ(REFERENCE_CONFIG_STRING, str);
}

//////////////////////////////////////////////////////////////////////////

TEST(ReflectionFundamentalTypeTest, DeserializeBool_False)
{
    const auto* type = GetType<bool>();

    bool obj;
    const ConfigValue value(false);
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(false, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeBool_True)
{
    const auto* type = GetType<bool>();

    bool obj;
    const ConfigValue value(true);
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(true, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt8)
{
    const auto* type = GetType<int8>();

    int8 obj;
    const ConfigValue value(static_cast<int8>(-35));
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(-35, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt16)
{
    const auto* type = GetType<int16>();

    int16 obj;
    const ConfigValue value(static_cast<int16>(-3255));
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(-3255, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt32)
{
    const auto* type = GetType<int32>();

    int32 obj;
    const ConfigValue value(-3253625);
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(-3253625, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeInt64)
{
    const auto* type = GetType<int64>();

    int64 obj;
    const ConfigValue value(static_cast<int64>(-10184017501ll));
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(-10184017501, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt8)
{
    const auto* type = GetType<uint8>();

    uint8 obj;
    const ConfigValue value(static_cast<uint8>(35u));
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(35u, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt16)
{
    const auto* type = GetType<uint16>();

    uint16 obj;
    const ConfigValue value(static_cast<uint16>(3255u));
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(3255u, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt32)
{
    const auto* type = GetType<uint32>();

    uint32 obj;
    const ConfigValue value(3253625u);
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(3253625u, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeUInt64)
{
    const auto* type = GetType<uint64>();

    uint64 obj;
    const ConfigValue value(static_cast<uint64>(10184017501ull));
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(10184017501ull, obj);
}


TEST(ReflectionFundamentalTypeTest, DeserializeFloat)
{
    const auto* type = GetType<float>();

    float obj;
    const ConfigValue value(1234.0f);
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(1234.0f, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeDouble)
{
    const auto* type = GetType<double>();

    double obj;
    const ConfigValue value(1234.0);
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ(1234.0, obj);
}

TEST(ReflectionFundamentalTypeTest, DeserializeString)
{
    const auto* type = GetType<String>();

    String obj;
    const ConfigValue value("abcd");
    SerializationContext context;
    ASSERT_TRUE(type->Deserialize(&obj, Config(), value, context));
    EXPECT_EQ("abcd", obj);
}