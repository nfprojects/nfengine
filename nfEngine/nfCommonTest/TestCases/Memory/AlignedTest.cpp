/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Aligned and AlignedAllocator classes.
 */

#include "PCH.hpp"
#include "nfCommon/Memory/Aligned.hpp"



using namespace NFE::Common;

namespace {

struct NFE_ALIGN(16) TestClass16 : public Aligned<16>
{
    int a;

    TestClass16() : a(0) { }
    explicit TestClass16(int val) : a(val) { }
};

struct NFE_ALIGN(64) TestClass64 : public Aligned<64>
{
    int a;

    TestClass64() : a(0) { }
    explicit TestClass64(int val) : a(val) { }
};

struct NFE_ALIGN(4096) TestClass4096 : public Aligned<4096>
{
    int a;

    TestClass4096() : a(0) { }
    explicit TestClass4096(int val) : a(val) { }
};

} // namespace


static_assert(alignof(TestClass16) == 16, "Invalid alignment of test class");
static_assert(alignof(TestClass64) == 64, "Invalid alignment of test class");
static_assert(alignof(TestClass4096) == 4096, "Invalid alignment of test class");


TEST(Aligned, AlignedObject)
{
    std::unique_ptr<TestClass16> testObject16(new TestClass16);
    std::unique_ptr<TestClass64> testObject64(new TestClass64);
    std::unique_ptr<TestClass4096> testObject4096(new TestClass4096);

    *testObject16 = TestClass16(123);
    *testObject64 = TestClass64(123);
    *testObject4096 = TestClass4096(123);

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

TEST(Aligned, AlignedArray)
{
    const size_t numElements = 100;

    std::unique_ptr<TestClass16[]> testObject16(new TestClass16[numElements]);
    std::unique_ptr<TestClass64[]> testObject64(new TestClass64[numElements]);
    std::unique_ptr<TestClass4096[]> testObject4096(new TestClass4096[numElements]);

    for (size_t i = 0; i < numElements; ++i)
    {
        testObject16[i] = TestClass16(123);
        testObject64[i] = TestClass64(123);
        testObject4096[i] = TestClass4096(123);
    }

    EXPECT_EQ(0, reinterpret_cast<size_t>(testObject16.get()) % 16);
    EXPECT_EQ(0, reinterpret_cast<size_t>(testObject64.get()) % 64);
    EXPECT_EQ(0, reinterpret_cast<size_t>(testObject4096.get()) % 4096);
}