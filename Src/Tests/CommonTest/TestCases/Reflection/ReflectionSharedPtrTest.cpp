#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"
#include "Engine/Common/Reflection/ReflectionUtils.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


namespace {

using TestSharedPtr = SharedPtr<TestBaseClass>;

} // namespace



class TestClassWithSharedPtrRecursion : public IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(TestClassWithSharedPtrRecursion)
public:
    SharedPtr<TestClassWithSharedPtrRecursion> child;
};

NFE_DEFINE_POLYMORPHIC_CLASS(TestClassWithSharedPtrRecursion)
{
    NFE_CLASS_MEMBER(child);
}
NFE_END_DEFINE_CLASS()



TEST(ReflectionTest, SharedPtr_Verify)
{
    const auto* type = GetType<TestSharedPtr>();
    ASSERT_NE(nullptr, type);

    EXPECT_TRUE("SharedPtr<TestBaseClass>" == type->GetName());
    EXPECT_EQ(TypeKind::SharedPtr, type->GetKind());
    EXPECT_EQ(sizeof(TestSharedPtr), type->GetSize());
    EXPECT_EQ(alignof(TestSharedPtr), type->GetAlignment());
}

TEST(ReflectionTest, SharedPtr_DefaultObject)
{
    const TestSharedPtr* defaultObjectPtr = GetDefaultObject<TestSharedPtr>();
    ASSERT_NE(nullptr, defaultObjectPtr);

    const TestSharedPtr& defaultObject = *defaultObjectPtr;
    EXPECT_EQ(nullptr, defaultObject.Get());
}

TEST(ReflectionTest, SharedPtr_Compare)
{
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr objNullptrA;
    TestSharedPtr objNullptrB;
    TestSharedPtr ptrA = MakeUniquePtr<TestBaseClass>();
    TestSharedPtr ptrB = MakeUniquePtr<TestBaseClass>();

    EXPECT_TRUE(type->Compare(&objNullptrA, &objNullptrA));
    EXPECT_TRUE(type->Compare(&objNullptrA, &objNullptrB));
    EXPECT_FALSE(type->Compare(&objNullptrA, &ptrA));
    EXPECT_TRUE(type->Compare(&ptrA, &ptrA));
    EXPECT_TRUE(type->Compare(&ptrA, &ptrB));

    ptrB->intVal = 123;
    EXPECT_FALSE(type->Compare(&ptrA, &ptrB));
}

TEST(ReflectionTest, SharedPtr_Clone)
{
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr ptrA = MakeUniquePtr<TestBaseClass>();
    ptrA->intVal = 123;
    ptrA->floatVal = 123.0f;

    TestSharedPtr ptrB;

    EXPECT_TRUE(type->Clone(&ptrB, &ptrA));
    ASSERT_TRUE(ptrB);
    EXPECT_NE(ptrA.Get(), ptrB.Get());

    EXPECT_TRUE(type->Compare(&ptrA, &ptrB));
}

TEST(ReflectionTest, SharedPtr_Serialize_Nullptr)
{
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr obj;
    String str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={}", str.Str());
}

TEST(ReflectionTest, SharedPtr_Serialize_Base)
{
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr obj = MakeSharedPtr<TestBaseClass>();
    obj->floatVal = 123.0f;
    obj->intVal = 321;

    String str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={__type=\"TestBaseClass\" __objID=0 __value={intVal=321 floatVal=123}}", str.Str());
}

TEST(ReflectionTest, SharedPtr_Serialize_DifferentObjects)
{
    const auto* type = GetType<SerializationTestClass>();

    TestSharedPtr subObjA = MakeSharedPtr<TestBaseClass>();
    subObjA->intVal = 123;

    TestSharedPtr subObjB = MakeSharedPtr<TestBaseClass>();
    subObjB->intVal = 456;

    SerializationTestClass obj;
    obj.sharedPtrA = subObjA;
    obj.sharedPtrB = subObjB;

    String str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={sharedPtrA={__type=\"TestBaseClass\" __objID=0 __value={intVal=123}} sharedPtrB={__type=\"TestBaseClass\" __objID=1 __value={intVal=456}}}", str.Str());
}


TEST(ReflectionTest, SharedPtr_Serialize_SharedObjects)
{
    const auto* type = GetType<SerializationTestClass>();

    TestSharedPtr sharedObj = MakeSharedPtr<TestBaseClass>();
    sharedObj->intVal = 123;

    SerializationTestClass obj;
    obj.sharedPtrA = sharedObj;
    obj.sharedPtrB = sharedObj;

    String str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={sharedPtrA={__type=\"TestBaseClass\" __objID=0 __value={intVal=123}} sharedPtrB={__objID=0}}", str.Str());
}

TEST(ReflectionTest, SharedPtr_Deserialize_Nullptr)
{
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj={}"));
    EXPECT_TRUE(obj == nullptr);
}

TEST(ReflectionTest, SharedPtr_Deserialize_MissingValueMarker)
{
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj, "obj={__type=\"TestBaseClass\"}"));
}

TEST(ReflectionTest, SharedPtr_Deserialize_MissingTypeMarker)
{
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj, "obj={__objID=0 __value={intVal=321 floatVal=123}}"));
}

TEST(ReflectionTest, SharedPtr_Deserialize_InvalidObjectID)
{
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj, "obj={__objID=1}"));
}

TEST(ReflectionTest, SharedPtr_Deserialize_InvalidType)
{
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj, "obj={__type=\"InvalidTypeName\"}"));
}

TEST(ReflectionTest, SharedPtr_Deserialize_Base)
{
    static_assert(std::is_constructible<TestBaseClass>::value, "TestBaseClass must be constructible");
    const auto* type = GetType<TestSharedPtr>();

    TestSharedPtr obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj={__type=\"TestBaseClass\" __value={intVal=111 floatVal=222.0 mPrivateBool=true}}"));

    ASSERT_TRUE(obj != nullptr);
    ASSERT_EQ(GetType<TestBaseClass>(), obj->GetDynamicType());
    EXPECT_EQ(111, obj->intVal);
    EXPECT_EQ(222.0f, obj->floatVal);
    EXPECT_EQ(true, obj->GetBool());
}

TEST(ReflectionTest, SharedPtr_Deserialize_DifferentObjects)
{
    const auto* type = GetType<SerializationTestClass>();

    SerializationTestClass obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj={sharedPtrA={__type=\"TestBaseClass\" __objID=0 __value={intVal=123}} sharedPtrB={__type=\"TestBaseClass\" __objID=1 __value={intVal=456}}}"));

    TestSharedPtr subObjA = Cast<TestBaseClass>(obj.sharedPtrA);
    TestSharedPtr subObjB = Cast<TestBaseClass>(obj.sharedPtrB);

    ASSERT_TRUE(obj.sharedPtrA != obj.sharedPtrB);
    ASSERT_TRUE(subObjA != nullptr);
    ASSERT_TRUE(subObjB != nullptr);
    EXPECT_EQ(123, subObjA->intVal);
    EXPECT_EQ(456, subObjB->intVal);
}


TEST(ReflectionTest, SharedPtr_Deserialize_SharedObjects)
{
    const auto* type = GetType<SerializationTestClass>();

    SerializationTestClass obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj={sharedPtrA={__type=\"TestBaseClass\" __objID=0 __value={intVal=123}} sharedPtrB={__objID=0}}"));

    TestSharedPtr subObj = Cast<TestBaseClass>(obj.sharedPtrA);

    ASSERT_TRUE(obj.sharedPtrA == obj.sharedPtrB);
    ASSERT_TRUE(subObj != nullptr);
    EXPECT_EQ(123, subObj->intVal);
}
