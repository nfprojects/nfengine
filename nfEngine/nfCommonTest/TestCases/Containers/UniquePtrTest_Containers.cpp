/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for UniquePtr (when used with other containers)
 */

#include "PCH.hpp"

#include "nfCommon/Containers/UniquePtr.hpp"
#include "nfCommon/Containers/DynArray.hpp"

using namespace NFE;
using namespace NFE::Common;

TEST(UniquePtr, UniquePtrOfUniquePtr)
{
    using Type = UniquePtr<int>;
    UniquePtr<Type> ptr = MakeUniquePtr<Type>(MakeUniquePtr<int>(1));

    ASSERT_EQ(1, **ptr);

    ptr->Reset();
    ASSERT_EQ(*ptr, nullptr);

    ptr.Reset();
    ASSERT_EQ(ptr, nullptr);
}

TEST(UniquePtr, UniquePtrOfDynArray)
{
    using Type = DynArray<int>;
    UniquePtr<Type> ptr = MakeUniquePtr<Type>();

    ptr->PushBack(1);
    ASSERT_EQ(1u, ptr->Size());

    ptr.Reset();
    ASSERT_EQ(ptr, nullptr);
}
