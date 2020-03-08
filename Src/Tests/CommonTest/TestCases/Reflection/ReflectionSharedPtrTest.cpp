#include "PCH.hpp"
#include "ReflectionTestCommon.hpp"


using namespace NFE;
using namespace NFE::Common;
using namespace NFE::RTTI;


namespace {

using TestUniquePtr = SharedPtr<TestBaseClass>;

} // namespace


TEST(ReflectionClassTest, SharedPtr_Verify)
{
    const auto* type = GetType<TestUniquePtr>();
    ASSERT_NE(nullptr, type);

    EXPECT_EQ("NFE::Common::SharedPtr<TestBaseClass>", type->GetName());
    EXPECT_EQ(TypeKind::SharedPtr, type->GetKind());
    EXPECT_EQ(sizeof(TestUniquePtr), type->GetSize());
    EXPECT_EQ(alignof(TestUniquePtr), type->GetAlignment());
}
