#include "PCH.hpp"
#include "nfCommon/Memory.hpp"
#include "nfCommon/Memory/DefaultAllocator.hpp"

#include <memory>
#include <limits>


using namespace NFE::Common;

using BufferUniquePtr = std::unique_ptr<char[], std::function<void(char*)>>;

TEST(MemoryTest, MallocSize)
{
    for (size_t size = 1; size < 10000000; size *= 2)
    {
        BufferUniquePtr buffer(reinterpret_cast<char*>(NFE_MALLOC(size, 1)),
                               [] (char* ptr) { NFE_FREE(ptr); });

        EXPECT_NE(nullptr, buffer.get());
        EXPECT_TRUE(MemoryCheck(buffer.get(), size));
    }
}

TEST(MemoryTest, MallocAlignment)
{
    size_t size = 4;

    for (size_t alignment = 1; alignment < 10000000; alignment *= 2)
    {
        BufferUniquePtr buffer(reinterpret_cast<char*>(NFE_MALLOC(size, alignment)),
                               [] (char* ptr) { NFE_FREE(ptr); });
        EXPECT_NE(nullptr, buffer.get());
        EXPECT_TRUE(MemoryCheck(buffer.get(), size));
        EXPECT_EQ(0, reinterpret_cast<size_t>(buffer.get()) & (alignment - 1));
    }
}

TEST(MemoryTest, MallocInvalidSize)
{
    size_t maxSize = std::numeric_limits<size_t>::max();

    EXPECT_EQ(nullptr, NFE_MALLOC(0, 1));
    EXPECT_EQ(nullptr, NFE_MALLOC(maxSize, 1));
}

TEST(MemoryTest, MallocInvalidAlignment)
{
    EXPECT_EQ(nullptr, NFE_MALLOC(1, 0));
    EXPECT_EQ(nullptr, NFE_MALLOC(1, 3));
    EXPECT_EQ(nullptr, NFE_MALLOC(1, 5));
}

TEST(MemoryTest, AccessValid)
{
    const size_t ARRAY_SIZE = 4096;

    std::unique_ptr<char[]> array(new char [ARRAY_SIZE]);
    std::unique_ptr<char> singleValue(new char);
    char arrayOnStack[ARRAY_SIZE];
    char singleValueOnStack;

    EXPECT_TRUE(MemoryCheck(array.get(), ARRAY_SIZE));
    EXPECT_TRUE(MemoryCheck(singleValue.get(), 1));
    EXPECT_TRUE(MemoryCheck(arrayOnStack, ARRAY_SIZE));
    EXPECT_TRUE(MemoryCheck(&singleValueOnStack));
}

TEST(MemoryTest, AccessInvalid)
{
    int* x = nullptr;
    int validValue = 123;
    EXPECT_FALSE(MemoryCheck(x));
    EXPECT_FALSE(MemoryCheck(&validValue, 1024 * 1024 * 64)); // stack won't have 64MB
}
