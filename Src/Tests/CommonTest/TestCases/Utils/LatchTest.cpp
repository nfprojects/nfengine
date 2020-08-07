/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Unit tests for Latch class.
 */

#include "PCH.hpp"
#include "Engine/Common/Utils/Latch.hpp"
#include "Engine/Common/System/Thread.hpp"


namespace {
    const unsigned int TIMEOUT = 500; /* ms */
} // namespace

using namespace NFE::Common;

TEST(Latch, Wait)
{
    Latch latch;
    Thread thread;
    
    ASSERT_TRUE(thread.Run([&latch]() {
        latch.Set();
    }));

    thread.Wait();
    latch.Wait();
}

TEST(Latch, WaitFor)
{
    Latch latch;
    Thread thread;

    ASSERT_TRUE(thread.Run([&latch]() {
        latch.Set();
    }));

    thread.Wait();
    ASSERT_TRUE(latch.Wait(TIMEOUT));
}

TEST(Latch, Timeout)
{
    Latch latch;
    ASSERT_FALSE(latch.Wait(TIMEOUT));
}
