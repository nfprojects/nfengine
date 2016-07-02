#include "PCH.hpp"
#include "nfCommon/Memory.hpp"

#include <memory>


using namespace NFE::Common;

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
