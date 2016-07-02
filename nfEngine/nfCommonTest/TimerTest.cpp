#include "PCH.hpp"
#include "nfCommon/Timer.hpp"

// required to test because of multi-platform sleep - std::this_thread::sleep_for
#include <chrono>
#include <thread>


namespace {
const double SLEEP_FOR_ACCURACY = 0.002; // 2 ms
const double TOLERANCE = 0.01; // 10 ms
} // namespace


class TimerTest : public testing::Test
{
protected:
    NFE::Common::Timer mTimer;
};


TEST_F(TimerTest, ImmediateStartStopTest)
{
    double result;

    mTimer.Start();
    result = mTimer.Stop();

    ASSERT_GE(result, 0.0);
}

TEST_F(TimerTest, BasicUseTest)
{
    const double expected = 1.0;
    double result;

    mTimer.Start();
    std::this_thread::sleep_for(std::chrono::seconds(static_cast<int>(expected)));
    result = mTimer.Stop();

    // make sure we got positive value
    ASSERT_GT(result, 0.0);

    // verify if correct time measured according to formula:
    //   -SLEEP_FOR_ACCURACY < result - expected < TOLERANCE
    result -= expected;
    ASSERT_GT(result, -SLEEP_FOR_ACCURACY);
    ASSERT_LT(result, TOLERANCE);
}

TEST_F(TimerTest, MultipleStopTest)
{
    double result1, result2;

    mTimer.Start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    result1 = mTimer.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    result2 = mTimer.Stop();

    ASSERT_GT(result1, 0.0);
    ASSERT_GT(result2, 0.0);
    ASSERT_GT(result2, result1);
}

TEST_F(TimerTest, MultipleStartTest)
{
    double result1, result2;

    mTimer.Start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    result1 = mTimer.Stop();

    mTimer.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    result2 = mTimer.Stop();

    ASSERT_GT(result1, 0.0);
    ASSERT_GT(result2, 0.0);
    ASSERT_GT(result1, result2);
}
