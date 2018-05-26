/**
 * @file
 * @author Witek902
 * @brief  Unit tests for Deque
 */

#include "PCH.hpp"
#include "nfCommon/Containers/Deque.hpp"

#include "TestClasses.hpp"

using namespace NFE;
using namespace NFE::Common;

TEST(Deque, Empty)
{
    Deque<uint32> deque;

    EXPECT_EQ(0u, deque.Size());
    EXPECT_TRUE(deque.Empty());
    EXPECT_FALSE(deque.PopBack());
    EXPECT_FALSE(deque.PopFront());
}

TEST(Deque, PushBack)
{
    const uint32 numElements = 1000u;
    Deque<uint32> deque;

    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_TRUE(deque.PushBack(i));
        ASSERT_EQ(i + 1u, deque.Size());
        ASSERT_FALSE(deque.Empty());
        ASSERT_EQ(i, deque.Back());
        ASSERT_EQ(0u, deque.Front());
    }
}

TEST(Deque, PushFront)
{
    const uint32 numElements = 1000u;
    Deque<uint32> deque;

    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_TRUE(deque.PushFront(i));
        ASSERT_EQ(i + 1u, deque.Size());
        ASSERT_FALSE(deque.Empty());
        ASSERT_EQ(0u, deque.Back());
        ASSERT_EQ(i, deque.Front());
    }
}

TEST(Deque, PushBack_PopFront)
{
    const uint32 numElements = 1000u;
    Deque<uint32> deque;

    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_TRUE(deque.PushBack(i));
    }

    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_EQ(numElements - i, deque.Size());
        ASSERT_FALSE(deque.Empty());
        ASSERT_EQ(numElements - 1, deque.Back());
        ASSERT_EQ(i, deque.Front());

        ASSERT_TRUE(deque.PopFront());
    }

    EXPECT_EQ(0u, deque.Size());
    EXPECT_TRUE(deque.Empty());
}

TEST(Deque, PushBack_PopBack)
{
    const uint32 numElements = 1000u;
    Deque<uint32> deque;

    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_TRUE(deque.PushBack(i));
    }

    for (uint32 i = 0; i < numElements; ++i)
    {
        ASSERT_EQ(numElements - i, deque.Size());
        ASSERT_FALSE(deque.Empty());
        ASSERT_EQ(numElements - 1 - i, deque.Back());
        ASSERT_EQ(0u, deque.Front());

        ASSERT_TRUE(deque.PopBack());
    }

    EXPECT_EQ(0u, deque.Size());
    EXPECT_TRUE(deque.Empty());
}

TEST(Deque, PushBack_PopFront_Asymmetric)
{
    const uint32 numIterations = 1000u;
    Deque<uint32> deque;

    for (uint32 i = 0; i < numIterations; ++i)
    {
        SCOPED_TRACE("i=" + std::to_string(i));

        ASSERT_TRUE(deque.PushBack(2 * i));
        ASSERT_TRUE(deque.PushBack(2 * i + 1));
        ASSERT_TRUE(deque.PopFront());

        ASSERT_EQ(i + 1, deque.Size());
        ASSERT_EQ(2 * i + 1, deque.Back());
        ASSERT_EQ(i + 1, deque.Front());
    }
}

TEST(Deque, PushFront_PopBack_Asymmetric)
{
    const uint32 numIterations = 1000u;
    Deque<uint32> deque;

    for (uint32 i = 0; i < numIterations; ++i)
    {
        SCOPED_TRACE("i=" + std::to_string(i));

        ASSERT_TRUE(deque.PushFront(2 * i));
        ASSERT_TRUE(deque.PushFront(2 * i + 1));
        ASSERT_TRUE(deque.PopBack());

        ASSERT_EQ(i + 1, deque.Size());
        ASSERT_EQ(i + 1, deque.Back());
        ASSERT_EQ(2 * i + 1, deque.Front());
    }
}