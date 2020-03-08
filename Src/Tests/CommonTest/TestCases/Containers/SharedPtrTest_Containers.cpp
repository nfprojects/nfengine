/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for SharedPtr (when used with other containers)
 */

#include "PCH.hpp"

#include "Engine/Common/Containers/SharedPtr.hpp"
#include "Engine/Common/Containers/DynArray.hpp"

using namespace NFE;
using namespace NFE::Common;

TEST(SharedPtr, SharedPtrOfSharedPtr)
{
    using Type = SharedPtr<int>;
    SharedPtr<Type> ptr = MakeSharedPtr<Type>(MakeSharedPtr<int>(1));

    ASSERT_EQ(1, **ptr);

    ptr->Reset();
    ASSERT_EQ(*ptr, nullptr);

    ptr.Reset();
    ASSERT_EQ(ptr, nullptr);
}

TEST(SharedPtr, SharedPtrOfDynArray)
{
    using Type = DynArray<int>;
    SharedPtr<Type> ptr = MakeSharedPtr<Type>();

    ptr->PushBack(1);
    ASSERT_EQ(1u, ptr->Size());

    ptr.Reset();
    ASSERT_EQ(ptr, nullptr);
}
