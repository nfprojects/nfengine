/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Unit tests for Latch class.
 */

#include "PCH.hpp"
#include "nfCommon/Latch.hpp"

#include <thread>

namespace {
    const unsigned int TIMEOUT = 500; /* ms */
} // namespace

using namespace NFE::Common;

TEST(Latch, Wait)
{
    Latch l;
    std::thread t([&l]() {
        l.Set();
    });

    t.join();
    ASSERT_NO_THROW(l.Wait());
}

TEST(Latch, WaitFor)
{
    Latch l;
    std::thread t([&l]() {
        l.Set();
    });

    t.join();
    ASSERT_TRUE(l.Wait(TIMEOUT));
}

TEST(Latch, Timeout)
{
    Latch l;
    ASSERT_FALSE(l.Wait(TIMEOUT));
}
