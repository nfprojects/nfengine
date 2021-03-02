#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Config/Config.hpp"
#include "Engine/Common/Reflection/SerializationContext.hpp"
#include "Engine/Common/Reflection/Types/ReflectionStaticBufferType.hpp"
#include "Engine/Common/Reflection/Types/ReflectionDynamicBufferType.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"
#include "Engine/Common/Memory/StaticBuffer.hpp"
#include "Engine/Common/Memory/DynamicBuffer.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


TEST(ReflectionBufferTest, Static_Verify)
{
    const auto* type = GetType<StaticBuffer>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("StaticBuffer", type->GetName());
    EXPECT_EQ(TypeKind::StaticBuffer, type->GetKind());
    EXPECT_EQ(sizeof(StaticBuffer), type->GetSize());
    EXPECT_EQ(alignof(StaticBuffer), type->GetAlignment());
}

TEST(ReflectionBufferTest, Dynamic_Verify)
{
    const auto* type = GetType<DynamicBuffer>();
    ASSERT_NE(nullptr, type);

    // check class type properties
    EXPECT_EQ("DynamicBuffer", type->GetName());
    EXPECT_EQ(TypeKind::DynamicBuffer, type->GetKind());
    EXPECT_EQ(sizeof(DynamicBuffer), type->GetSize());
    EXPECT_EQ(alignof(DynamicBuffer), type->GetAlignment());
}

TEST(ReflectionBufferTest, Static_SerializeBinary_Empty)
{
    const auto* type = GetType<StaticBuffer>();

    DynamicBuffer buffer;
    SerializationContext context;
    {
        StaticBuffer obj;
        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        StaticBuffer readObj;
        ASSERT_TRUE(readObj.Set(1, "a"));

        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));

        ASSERT_EQ(0u, readObj.Size());
    }
}

TEST(ReflectionBufferTest, Static_SerializeBinary_NonEmpty)
{
    const auto* type = GetType<StaticBuffer>();

    DynamicBuffer buffer;
    SerializationContext context;
    {
        StaticBuffer obj;
        ASSERT_TRUE(obj.Set(4, "test"));

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        StaticBuffer readObj;
        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));

        ASSERT_EQ(4u, readObj.Size());
        EXPECT_EQ(0, memcmp("test", readObj.Data(), 4));
    }
}

TEST(ReflectionBufferTest, Dynamic_SerializeBinary_Empty)
{
    const auto* type = GetType<StaticBuffer>();

    DynamicBuffer buffer;
    SerializationContext context;
    {
        DynamicBuffer obj;
        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        DynamicBuffer readObj;
        ASSERT_TRUE(readObj.Resize(1, "a"));

        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));

        ASSERT_EQ(0u, readObj.Size());
    }
}

TEST(ReflectionBufferTest, Dynamic_SerializeBinary_NonEmpty)
{
    const auto* type = GetType<StaticBuffer>();

    DynamicBuffer buffer;
    SerializationContext context;
    {
        DynamicBuffer obj;
        ASSERT_TRUE(obj.Resize(4, "test"));

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        DynamicBuffer readObj;
        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));

        ASSERT_EQ(4u, readObj.Size());
        EXPECT_EQ(0, memcmp("test", readObj.Data(), 4));
    }
}
