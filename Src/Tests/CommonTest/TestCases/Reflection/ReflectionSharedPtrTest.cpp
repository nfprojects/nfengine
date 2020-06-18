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

    EXPECT_TRUE("NFE::Common::SharedPtr<TestBaseClass>" == type->GetName());
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
    EXPECT_STREQ("obj=0", str.Str());
}