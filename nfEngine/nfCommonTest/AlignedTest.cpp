/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Aligned and AlignedAllocator classes.
 */

#include "PCH.hpp"
#include "../nfCommon/Aligned.hpp"
#include "../nfCommon/ClassRegister.hpp"

using namespace NFE::Common;

namespace {

struct TestClass16
{
    NFE_DECLARE_CLASS;
    int a;
};

struct TestClass64
{
    NFE_DECLARE_CLASS;
    int a;
};

struct TestClass4096
{
    NFE_DECLARE_CLASS;
    int a;
};

} // namespace

NFE_DEFINE_CLASS(TestClass16, 16, ClassAllocatorType::Generic);
NFE_DEFINE_CLASS(TestClass64, 64, ClassAllocatorType::Generic);
NFE_DEFINE_CLASS(TestClass4096, 4096, ClassAllocatorType::Generic);


TEST(Aligned, AlignedObject)
{
    std::unique_ptr<TestClass16> testObject16(new TestClass16);
    std::unique_ptr<TestClass64> testObject64(new TestClass64);
    std::unique_ptr<TestClass4096> testObject4096(new TestClass4096);

    EXPECT_EQ(0, reinterpret_cast<size_t>(testObject16.get()) % 16);
    EXPECT_EQ(0, reinterpret_cast<size_t>(testObject64.get()) % 64);
    EXPECT_EQ(0, reinterpret_cast<size_t>(testObject4096.get()) % 4096);
}

TEST(Aligned, AlignedContainer)
{
    std::vector<int, AlignedAllocator<int, 16>> vector16;
    std::vector<int, AlignedAllocator<int, 64>> vector64;
    std::vector<int, AlignedAllocator<int, 4096>> vector4096;

    vector16.push_back(1);
    vector64.push_back(2);
    vector4096.push_back(3);

    EXPECT_EQ(0, reinterpret_cast<size_t>(vector16.data()) % 16);
    EXPECT_EQ(0, reinterpret_cast<size_t>(vector64.data()) % 64);
    EXPECT_EQ(0, reinterpret_cast<size_t>(vector4096.data()) % 4096);
}
