#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Reflection/SerializationContext.hpp"
#include "Engine/Common/Reflection/ReflectionUnitTestHelper.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"
#include "Engine/Common/Memory/Buffer.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;

///////////////////////////////////////////////////////////////////////////////////////////////////

class TestClass_IncompatibleType_Before
{
    NFE_DECLARE_CLASS(TestClass_IncompatibleType_Before)
public:
    int32 a = 0;
    int32 b = 0;
    int32 c = 0;
};

NFE_DEFINE_CLASS(TestClass_IncompatibleType_Before)
{
    NFE_CLASS_MEMBER(a);
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

class TestClass_IncompatibleType_After
{
    NFE_DECLARE_CLASS(TestClass_IncompatibleType_After)
public:
    int32 a = 0;
    float b = 1.0f; // was int32
    int32 c = 0;
};

NFE_DEFINE_CLASS(TestClass_IncompatibleType_After)
{
    NFE_CLASS_MEMBER(a);
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

TEST(ReflectionTypeMismatchTest, IncompatibleType)
{
    Buffer buffer;
    SerializationContext context;

    {
        TestClass_IncompatibleType_Before obj;
        obj.a = 1;
        obj.b = 2;
        obj.c = 3;

        const auto* typeBefore = GetType<TestClass_IncompatibleType_Before>();
        ASSERT_NE(nullptr, typeBefore);

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(typeBefore, &obj, stream, context));
    }

    {
        const auto* typeAfter = GetType<TestClass_IncompatibleType_After>();
        ASSERT_NE(nullptr, typeAfter);

        TestClass_IncompatibleType_After readObj;
        BufferInputStream stream(buffer);
        ASSERT_TRUE(typeAfter->DeserializeBinary(&readObj, stream, context));

        EXPECT_EQ(1, readObj.a);
        EXPECT_EQ(1.0f, readObj.b);
        EXPECT_EQ(3, readObj.c);

        const auto& mismatchedTypes = context.GetMemberTypeMismatchInfos();
        ASSERT_EQ(1u, mismatchedTypes.Size());
        EXPECT_EQ(MemberPath("b"), mismatchedTypes[0].path);
        ASSERT_EQ(GetType<int32>(), mismatchedTypes[0].readObject.GetType());
        EXPECT_EQ(2, mismatchedTypes[0].readObject.Get<int32>());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class TestClass_CompatibleFundamentalType_Before
{
    NFE_DECLARE_CLASS(TestClass_CompatibleFundamentalType_Before)
public:
    int32 a = 0;
    int32 b = 0;
    int32 c = 0;
};

NFE_DEFINE_CLASS(TestClass_CompatibleFundamentalType_Before)
{
    NFE_CLASS_MEMBER(a);
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

class TestClass_CompatibleFundamentalType_After
{
    NFE_DECLARE_CLASS(TestClass_CompatibleFundamentalType_After)
public:
    int32 a = 0;
    int64 b = 0; // was int32
    int32 c = 0;
};

NFE_DEFINE_CLASS(TestClass_CompatibleFundamentalType_After)
{
    NFE_CLASS_MEMBER(a);
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

TEST(ReflectionTypeMismatchTest, CompatibleFundamentalType)
{
    Buffer buffer;
    SerializationContext context;

    {
        TestClass_CompatibleFundamentalType_Before obj;
        obj.a = 1;
        obj.b = 2;
        obj.c = 3;

        const auto* typeBefore = GetType<TestClass_CompatibleFundamentalType_Before>();
        ASSERT_NE(nullptr, typeBefore);

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(typeBefore, &obj, stream, context));
    }

    {
        const auto* typeAfter = GetType<TestClass_CompatibleFundamentalType_After>();
        ASSERT_NE(nullptr, typeAfter);

        TestClass_CompatibleFundamentalType_After readObj;
        readObj.a = 0x12345678;
        readObj.b = 0x1234567812345678;
        readObj.c = 0x12345678;

        BufferInputStream stream(buffer);
        ASSERT_TRUE(typeAfter->DeserializeBinary(&readObj, stream, context));

        EXPECT_EQ(1, readObj.a);
        EXPECT_EQ(2, readObj.b);
        EXPECT_EQ(3, readObj.c);

        EXPECT_EQ(0u, context.GetMemberTypeMismatchInfos().Size());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class TestClass_IncompatibleNativeArrayType_Before
{
    NFE_DECLARE_CLASS(TestClass_IncompatibleNativeArrayType_Before)
public:
    int32 b[3] = { 0, 0, 0 };
    DynArray<int32> c[3];
};

NFE_DEFINE_CLASS(TestClass_IncompatibleNativeArrayType_Before)
{
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

class TestClass_IncompatibleNativeArrayType_After
{
    NFE_DECLARE_CLASS(TestClass_IncompatibleNativeArrayType_After)
public:
    int32 b[2] = { 0, 0 }; // was int32[2]
    DynArray<int32> c[2]; // was DynArray<int32>[3]
};

NFE_DEFINE_CLASS(TestClass_IncompatibleNativeArrayType_After)
{
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

TEST(ReflectionTypeMismatchTest, IncompatibleNativeArrayType)
{
    Buffer buffer;
    SerializationContext context;

    {
        TestClass_IncompatibleNativeArrayType_Before obj;
        obj.b[0] = 123;
        obj.b[1] = 456;
        obj.b[2] = 789;
        obj.c[0].PushBack(123);
        obj.c[1].PushBack(456);
        obj.c[2].PushBack(789);

        const auto* typeBefore = GetType<TestClass_IncompatibleNativeArrayType_Before>();
        ASSERT_NE(nullptr, typeBefore);

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(typeBefore, &obj, stream, context));
    }

    {
        const auto* typeAfter = GetType<TestClass_IncompatibleNativeArrayType_After>();
        ASSERT_NE(nullptr, typeAfter);

        TestClass_IncompatibleNativeArrayType_After readObj;
        readObj.b[0] = 1;
        readObj.b[1] = 2;
        readObj.c[0].PushBack(1);
        readObj.c[0].PushBack(2);
        readObj.c[1].PushBack(1);
        readObj.c[1].PushBack(2);

        BufferInputStream stream(buffer);
        ASSERT_TRUE(typeAfter->DeserializeBinary(&readObj, stream, context));

        EXPECT_EQ(1, readObj.b[0]);
        EXPECT_EQ(2, readObj.b[1]);

        ASSERT_EQ(2u, readObj.c[0].Size());
        EXPECT_EQ(1, readObj.c[0][0]);
        EXPECT_EQ(2, readObj.c[0][1]);

        ASSERT_EQ(2u, readObj.c[1].Size());
        EXPECT_EQ(1, readObj.c[1][0]);
        EXPECT_EQ(2, readObj.c[1][1]);

        const auto& mismatchedTypes = context.GetMemberTypeMismatchInfos();
        ASSERT_EQ(2u, mismatchedTypes.Size());
        {
            EXPECT_EQ(MemberPath("b"), mismatchedTypes[0].path);
            ASSERT_EQ(GetType<int32[3]>(), mismatchedTypes[0].readObject.GetType());
            const auto& readArray = mismatchedTypes[0].readObject.Get<int32[3]>();
            EXPECT_EQ(123, readArray[0]);
            EXPECT_EQ(456, readArray[1]);
            EXPECT_EQ(789, readArray[2]);
        }
        {
            EXPECT_EQ(MemberPath("c"), mismatchedTypes[1].path);
            ASSERT_EQ(GetType<DynArray<int32>[3]>(), mismatchedTypes[1].readObject.GetType());
            const auto& readArray = mismatchedTypes[1].readObject.Get<DynArray<int32>[3]>();
            ASSERT_EQ(1u, readArray[0].Size());
            ASSERT_EQ(1u, readArray[1].Size());
            ASSERT_EQ(1u, readArray[2].Size());
            EXPECT_EQ(123, readArray[0][0]);
            EXPECT_EQ(456, readArray[1][0]);
            EXPECT_EQ(789, readArray[2][0]);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////

class TestClass_CompatibleNativeArrayType_Before
{
    NFE_DECLARE_CLASS(TestClass_CompatibleNativeArrayType_Before)
public:
    int32 a = 0;
    int32 b[2] = { 0, 0 };
    DynArray<int32> c[2];
};

NFE_DEFINE_CLASS(TestClass_CompatibleNativeArrayType_Before)
{
    NFE_CLASS_MEMBER(a);
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

class TestClass_CompatibleNativeArrayType_After
{
    NFE_DECLARE_CLASS(TestClass_CompatibleNativeArrayType_After)
public:
    int32 a = 0;
    int32 b[5] = { 0, 0, 0, 0, 0 }; // was int32[2]
    DynArray<int32> c[5]; // was DynArray<int32>[2]
};

NFE_DEFINE_CLASS(TestClass_CompatibleNativeArrayType_After)
{
    NFE_CLASS_MEMBER(a);
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

TEST(ReflectionTypeMismatchTest, CompatibleNativeArrayType)
{
    Buffer buffer;
    SerializationContext context;

    {
        TestClass_CompatibleNativeArrayType_Before obj;
        obj.a = 1;
        obj.b[0] = 12345;
        obj.b[1] = 67890;
        obj.c[0].PushBack(123);
        obj.c[1].PushBack(456);

        const auto* typeBefore = GetType<TestClass_CompatibleNativeArrayType_Before>();
        ASSERT_NE(nullptr, typeBefore);

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(typeBefore, &obj, stream, context));
    }

    {
        const auto* typeAfter = GetType<TestClass_CompatibleNativeArrayType_After>();
        ASSERT_NE(nullptr, typeAfter);

        TestClass_CompatibleNativeArrayType_After readObj;
        readObj.a = 0x12345678;
        readObj.b[0] = 1;
        readObj.b[1] = 2;
        readObj.b[2] = 3;
        readObj.b[3] = 4;
        readObj.b[4] = 5;
        readObj.c[0].PushBack(1);
        readObj.c[0].PushBack(2);
        readObj.c[1].PushBack(1);
        readObj.c[1].PushBack(2);

        BufferInputStream stream(buffer);
        ASSERT_TRUE(typeAfter->DeserializeBinary(&readObj, stream, context));

        EXPECT_EQ(1, readObj.a);
        EXPECT_EQ(12345, readObj.b[0]);
        EXPECT_EQ(67890, readObj.b[1]);

        ASSERT_EQ(1u, readObj.c[0].Size());
        EXPECT_EQ(123, readObj.c[0][0]);

        ASSERT_EQ(1u, readObj.c[1].Size());
        EXPECT_EQ(456, readObj.c[1][0]);

        EXPECT_EQ(0u, context.GetMemberTypeMismatchInfos().Size());
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////

class TestClass_NativeArrayToDynArray_Before
{
    NFE_DECLARE_CLASS(TestClass_NativeArrayToDynArray_Before)
public:
    int32 a = 0;
    int32 b[2] = { 0, 0 };
    DynArray<int32> c[2];
};

NFE_DEFINE_CLASS(TestClass_NativeArrayToDynArray_Before)
{
    NFE_CLASS_MEMBER(a);
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

class TestClass_NativeArrayToDynArray_After
{
    NFE_DECLARE_CLASS(TestClass_NativeArrayToDynArray_After)
public:
    int32 a = 0;
    DynArray<int32> b; // was int32[2]
    DynArray<DynArray<int32>> c; // was DynArray<int32>[2]
};

NFE_DEFINE_CLASS(TestClass_NativeArrayToDynArray_After)
{
    NFE_CLASS_MEMBER(a);
    NFE_CLASS_MEMBER(b);
    NFE_CLASS_MEMBER(c);
}
NFE_END_DEFINE_CLASS()

TEST(ReflectionTypeMismatchTest, NativeArrayToDynArray)
{
    Buffer buffer;
    SerializationContext context;

    {
        TestClass_NativeArrayToDynArray_Before obj;
        obj.a = 1;
        obj.b[0] = 12345;
        obj.b[1] = 67890;
        obj.c[0].PushBack(123);
        obj.c[1].PushBack(456);

        const auto* typeBefore = GetType<TestClass_NativeArrayToDynArray_Before>();
        ASSERT_NE(nullptr, typeBefore);

        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(typeBefore, &obj, stream, context));
    }

    {
        const auto* typeAfter = GetType<TestClass_NativeArrayToDynArray_After>();
        ASSERT_NE(nullptr, typeAfter);

        TestClass_NativeArrayToDynArray_After readObj;

        BufferInputStream stream(buffer);
        ASSERT_TRUE(typeAfter->DeserializeBinary(&readObj, stream, context));

        EXPECT_EQ(1, readObj.a);

        ASSERT_EQ(2u, readObj.b.Size());
        EXPECT_EQ(12345, readObj.b[0]);
        EXPECT_EQ(67890, readObj.b[1]);

        ASSERT_EQ(2u, readObj.c.Size());
        ASSERT_EQ(1u, readObj.c[0].Size());
        EXPECT_EQ(123, readObj.c[0][0]);
        ASSERT_EQ(1u, readObj.c[1].Size());
        EXPECT_EQ(456, readObj.c[1][0]);

        EXPECT_EQ(0u, context.GetMemberTypeMismatchInfos().Size());
    }
}
