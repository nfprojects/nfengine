#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Reflection/SerializationContext.hpp"
#include "Engine/Common/Utils/Stream/BufferOutputStream.hpp"
#include "Engine/Common/Utils/Stream/BufferInputStream.hpp"
#include "Engine/Common/Memory/Buffer.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


namespace {

using TestUniquePtr = UniquePtr<TestBaseClass>;

} // namespace


TEST(ReflectionUniquePtrTest, Verify)
{
    const auto* type = GetType<TestUniquePtr>();
    ASSERT_NE(nullptr, type);

    EXPECT_TRUE("NFE::Common::UniquePtr<TestBaseClass>" == type->GetName());
    EXPECT_EQ(TypeKind::UniquePtr, type->GetKind());
    EXPECT_EQ(sizeof(TestUniquePtr), type->GetSize());
    EXPECT_EQ(alignof(TestUniquePtr), type->GetAlignment());
}

TEST(ReflectionUniquePtrTest, Compare)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr objNullptrA;
    TestUniquePtr objNullptrB;
    TestUniquePtr ptrA = MakeUniquePtr<TestBaseClass>();
    TestUniquePtr ptrB = MakeUniquePtr<TestBaseClass>();

    EXPECT_TRUE(type->Compare(&objNullptrA, &objNullptrA));
    EXPECT_TRUE(type->Compare(&objNullptrA, &objNullptrB));
    EXPECT_FALSE(type->Compare(&objNullptrA, &ptrA));
    EXPECT_TRUE(type->Compare(&ptrA, &ptrA));
    EXPECT_TRUE(type->Compare(&ptrA, &ptrB));

    ptrB->intVal = 123;
    EXPECT_FALSE(type->Compare(&ptrA, &ptrB));
}

TEST(ReflectionUniquePtrTest, Clone)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr ptrA = MakeUniquePtr<TestBaseClass>();
    ptrA->intVal = 123;
    ptrA->floatVal = 123.0f;

    TestUniquePtr ptrB;

    EXPECT_TRUE(type->Clone(&ptrB, &ptrA));
    ASSERT_TRUE(ptrB);
    EXPECT_NE(ptrA.Get(), ptrB.Get());

    EXPECT_TRUE(type->Compare(&ptrA, &ptrB));
}

TEST(ReflectionUniquePtrTest, Serialize_Nullptr)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    String str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj=0", str.Str());
}

TEST(ReflectionUniquePtrTest, Serialize_Base)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj = MakeUniquePtr<TestBaseClass>();
    obj->floatVal = 123.0f;
    obj->intVal = 321;

    String str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={__type=\"TestBaseClass\" intVal=321 floatVal=123 mPrivateBool=false}", str.Str());
}

TEST(ReflectionUniquePtrTest, Deserialize_Nullptr)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj=0"));
    EXPECT_TRUE(obj == nullptr);
}

TEST(ReflectionUniquePtrTest, Deserialize_InvalidType)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj, "obj={__type=\"InvalidTypeName\"}"));
}

TEST(ReflectionUniquePtrTest, Deserialize_NonRelatedType)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj, "obj={__type=\"TestClassWithFundamentalMembers\"}"));
}

TEST(ReflectionUniquePtrTest, Deserialize_Base)
{
    static_assert(std::is_constructible<TestBaseClass>::value, "TestBaseClass must be constructible");
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj={__type=\"TestBaseClass\" intVal=111 floatVal=222.0 mPrivateBool=true}"));

    ASSERT_TRUE(obj != nullptr);
    ASSERT_EQ(GetType<TestBaseClass>(), obj->GetDynamicType());
    EXPECT_EQ(111, obj->intVal);
    EXPECT_EQ(222.0f, obj->floatVal);
    EXPECT_EQ(true, obj->GetBool());
}

TEST(ReflectionUniquePtrTest, Deserialize_Child)
{
    static_assert(std::is_constructible<TestChildClassA>::value, "TestChildClassA must be constructible");
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj={__type=\"TestChildClassA\" intVal=444 floatVal=777.0 mPrivateBool=true foo=567}"));

    ASSERT_TRUE(obj != nullptr);

    // verify type of deserialized object under the pointer
    ASSERT_EQ(GetType<TestChildClassA>(), obj->GetDynamicType());

    const TestChildClassA* typedPtr = static_cast<const TestChildClassA*>(obj.Get());
    EXPECT_EQ(444, typedPtr->intVal);
    EXPECT_EQ(777.0f, typedPtr->floatVal);
    EXPECT_EQ(true, typedPtr->GetBool());
    EXPECT_EQ(567, typedPtr->foo);
}

TEST(ReflectionUniquePtrTest, SerializeBinary_Nullptr)
{
    const auto* type = GetType<TestUniquePtr>();

    Buffer buffer;
    SerializationContext context;
    {
        TestUniquePtr obj;
        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        TestUniquePtr readObj;
        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));
        EXPECT_EQ(nullptr, readObj.Get());
    }
}

TEST(ReflectionUniquePtrTest, SerializeBinary)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj = MakeUniquePtr<TestBaseClass>();
    obj->floatVal = 123.0f;
    obj->intVal = 321;

    Buffer buffer;
    SerializationContext context;
    {
        BufferOutputStream stream(buffer);
        ASSERT_TRUE(helper::SerializeObject(type, &obj, stream, context));
    }
    {
        TestUniquePtr readObj;
        BufferInputStream stream(buffer);
        ASSERT_TRUE(type->DeserializeBinary(&readObj, stream, context));
        EXPECT_NE(nullptr, readObj.Get());
        EXPECT_NE(obj.Get(), readObj.Get());
        EXPECT_EQ(123.0f, readObj->floatVal);
        EXPECT_EQ(321, readObj->intVal);
    }
}