/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Aligned and AlignedAllocator classes.
 */

#include "PCH.hpp"
#include "nfCommon/Memory/Aligned.hpp"

#include <memory>


using namespace NFE::Common;

namespace {

struct TestClass16 : public Aligned<16>
{
    int a;
};

struct TestClass64 : public Aligned<64>
{
    int a;
};

struct TestClass4096 : public Aligned<4096>
{
    int a;
};

} // namespace

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
