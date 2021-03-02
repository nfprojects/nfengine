#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Reflection/ReflectionUtils.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"
#include "Engine/Common/Reflection/SerializationContext.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"
#include "Engine/Common/Memory/DynamicBuffer.hpp"

using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;

namespace {

using TestStaticArray = StaticArray<int32,3>;

} // namespace

TEST(ReflectionStaticArrayTest, Verify)
{
    const auto* type = GetType<TestStaticArray>();
    ASSERT_NE(nullptr, type);

    EXPECT_TRUE("StaticArray<NFE::int32,3>" == type->GetName());
    EXPECT_EQ(TypeKind::Array, type->GetKind());
    EXPECT_EQ(sizeof(TestStaticArray), type->GetSize());
    EXPECT_EQ(alignof(TestStaticArray), type->GetAlignment());
}

TEST(ReflectionStaticArrayTest, DefaultObject)
{
    const TestStaticArray* defaultObjectPtr = GetDefaultObject<TestStaticArray>();
    ASSERT_NE(nullptr, defaultObjectPtr);

    const TestStaticArray& defaultObject = *defaultObjectPtr;
    EXPECT_EQ(0u, defaultObject.Size());
}

TEST(ReflectionStaticArrayTest, Compare)
{
    const auto* type = GetType<TestStaticArray>();

    TestStaticArray emptyArrayA;
    TestStaticArray emptyArrayB;

    TestStaticArray arrayA;
    arrayA.PushBack(1);

    TestStaticArray arrayB;
    arrayB.PushBack(1);

    TestStaticArray arrayC;
    arrayC.PushBack(1);
    arrayC.PushBack(2);

    EXPECT_TRUE(type->Compare(&emptyArrayA, &emptyArrayB));
    EXPECT_TRUE(type->Compare(&emptyArrayA, &emptyArrayA));
    EXPECT_TRUE(type->Compare(&arrayA, &arrayB));
    EXPECT_FALSE(type->Compare(&emptyArrayA, &arrayA));
    EXPECT_FALSE(type->Compare(&arrayA, &emptyArrayA));
    EXPECT_FALSE(type->Compare(&arrayA, &arrayC));
    EXPECT_FALSE(type->Compare(&arrayC, &arrayA));
}

TEST(ReflectionStaticArrayTest, Resize)
{
    const auto* type = GetType<TestStaticArray>();

    TestStaticArray array;
    array.PushBack(123);

    EXPECT_TRUE(type->ResizeArray(&array, 3u));
    ASSERT_EQ(3u, array.Size());

    EXPECT_EQ(123, array[0]);
    EXPECT_EQ(0, array[1]);
    EXPECT_EQ(0, array[2]);

    EXPECT_TRUE(type->ResizeArray(&array, 2u));
    ASSERT_EQ(2u, array.Size());

    EXPECT_TRUE(type->ResizeArray(&array, 0u));
    ASSERT_EQ(0u, array.Size());
}

TEST(ReflectionStaticArrayTest, Clone_Empty)
{
    const auto* type = GetType<TestStaticArray>();

    TestStaticArray arrayA;

    TestStaticArray arrayB;
    arrayB.PushBack(1);
    EXPECT_EQ(1u, arrayB.Size());

    EXPECT_TRUE(type->Clone(&arrayB, &arrayA));
    EXPECT_EQ(0u, arrayB.Size());
}

TEST(ReflectionStaticArrayTest, Clone_SingleElement)
{
    const auto* type = GetType<TestStaticArray>();

    TestStaticArray arrayA;
    arrayA.PushBack(123);

    TestStaticArray arrayB;
    arrayB.PushBack(1);
    arrayB.PushBack(2);
    EXPECT_EQ(2u, arrayB.Size());

    EXPECT_TRUE(type->Clone(&arrayB, &arrayA));
    ASSERT_EQ(1u, arrayB.Size());
    EXPECT_EQ(123, arrayB[0]);
}

TEST(ReflectionStaticArrayTest, SerializeBinary_Empty)
{
    const auto* type = GetType<TestStaticArray>();

    DynamicBuffer buffer;
    SerializationContext context;
    {
        TestStaticArray obj;
        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        TestStaticArray readObj;
        readObj.PushBack(1);

        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));
        EXPECT_EQ(0u, readObj.Size());
    }
}

TEST(ReflectionStaticArrayTest, SerializeBinary)
{
    const auto* type = GetType<TestStaticArray>();

    DynamicBuffer buffer;
    SerializationContext context;
    {
        TestStaticArray obj;
        obj.PushBack(123);
        obj.PushBack(456);
        obj.PushBack(789);

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        TestStaticArray readObj;
        readObj.PushBack(1);

        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));
        ASSERT_EQ(3u, readObj.Size());

        EXPECT_EQ(123, readObj[0]);
        EXPECT_EQ(456, readObj[1]);
        EXPECT_EQ(789, readObj[2]);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class TestClassWithNestedStaticArrays
{
    NFE_DECLARE_CLASS(TestClassWithNestedStaticArrays)
public:
    StaticArray<StaticArray<int32, 3>, 4> a;
    StaticArray<int32, 4> b[2];
};

NFE_DEFINE_CLASS(TestClassWithNestedStaticArrays)
{
    NFE_CLASS_MEMBER(a);
    NFE_CLASS_MEMBER(b);
}
NFE_END_DEFINE_CLASS()

TEST(ReflectionStaticArrayTest, NestedStaticArrays)
{
    const auto* type = GetType<TestClassWithNestedStaticArrays>();
    ASSERT_NE(nullptr, type);

    DynamicBuffer buffer;
    SerializationContext context;

    {
        TestClassWithNestedStaticArrays obj;
        obj.a.PushBack(StaticArray<int32,3>());
        obj.a[0].PushBack(1);
        obj.a[0].PushBack(2);
        obj.b[0].PushBack(3);
        obj.b[1].PushBack(4);

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }

    {
        TestClassWithNestedStaticArrays readObj;
        // push some random data (they should be cleared on Deserialize)
        readObj.a.Resize(3);
        readObj.a[0].PushBack(10);
        readObj.a[0].PushBack(20);
        readObj.a[1].PushBack(10);
        readObj.a[1].PushBack(20);
        readObj.b[0].PushBack(10);
        readObj.b[0].PushBack(20);
        readObj.b[1].PushBack(10);
        readObj.b[1].PushBack(20);

        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));

        ASSERT_EQ(1u, readObj.a.Size());
        ASSERT_EQ(2u, readObj.a[0].Size());
        ASSERT_EQ(1u, readObj.b[0].Size());
        ASSERT_EQ(1u, readObj.b[1].Size());

        EXPECT_EQ(1, readObj.a[0][0]);
        EXPECT_EQ(2, readObj.a[0][1]);
        EXPECT_EQ(3, readObj.b[0][0]);
        EXPECT_EQ(4, readObj.b[1][0]);
    }
}
