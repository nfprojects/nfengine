#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


namespace {

using TestUniquePtr = UniquePtr<TestBaseClass>;

} // namespace


TEST(ReflectionClassTest, UniquePtr_Verify)
{
    const auto* type = GetType<TestUniquePtr>();
    ASSERT_NE(nullptr, type);

    EXPECT_STREQ("NFE::Common::UniquePtr<TestBaseClass>", type->GetName());
    EXPECT_EQ(TypeKind::UniquePtr, type->GetKind());
    EXPECT_EQ(sizeof(TestUniquePtr), type->GetSize());
    EXPECT_EQ(alignof(TestUniquePtr), type->GetAlignment());
}

TEST(ReflectionClassTest, UniquePtr_Serialize_Nullptr)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    std::string str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj=0", str.c_str());
}

TEST(ReflectionClassTest, UniquePtr_Serialize_Base)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj = MakeUniquePtr<TestBaseClass>();
    obj->floatVal = 123.0f;
    obj->intVal = 321;

    std::string str;
    ASSERT_TRUE(helper::SerializeObject(type, &obj, str));
    EXPECT_STREQ("obj={__type=\"TestBaseClass\" intVal=321 floatVal=123 mPrivateBool=false}", str.c_str());
}

TEST(ReflectionClassTest, UniquePtr_Deserialize_Nullptr)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    ASSERT_TRUE(helper::DeserializeObject(type, &obj, "obj=0"));
    EXPECT_TRUE(obj == nullptr);
}

TEST(ReflectionClassTest, UniquePtr_Deserialize_InvalidType)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj, "obj={__type=\"InvalidTypeName\"}"));
}

TEST(ReflectionClassTest, UniquePtr_Deserialize_NonRelatedType)
{
    const auto* type = GetType<TestUniquePtr>();

    TestUniquePtr obj;
    ASSERT_FALSE(helper::DeserializeObject(type, &obj, "obj={__type=\"TestClassWithFundamentalMembers\"}"));
}

TEST(ReflectionClassTest, UniquePtr_Deserialize_Base)
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

TEST(ReflectionClassTest, UniquePtr_Deserialize_Child)
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

