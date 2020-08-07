#include "PCH.hpp"
#include "Engine/Common/System/Timer.hpp"
#include "Engine/Common/System/Thread.hpp"


namespace {
const double SLEEP_FOR_ACCURACY = 0.002; // 2 ms
const double TOLERANCE = 0.01; // 10 ms
} // namespace

using namespace NFE;
using namespace Common;

class TimerTest : public testing::Test
{
protected:
    Timer mTimer;
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
    const double expected = 0.5;
    double result;

    mTimer.Start();
    Thread::SleepCurrentThread(0.5);
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
    Thread::SleepCurrentThread(1.0);
    result1 = mTimer.Stop();
    Thread::SleepCurrentThread(0.5);
    result2 = mTimer.Stop();

    ASSERT_GT(result1, 0.0);
    ASSERT_GT(result2, 0.0);
    ASSERT_GT(result2, result1);
}

TEST_F(TimerTest, MultipleStartTest)
{
    double result1, result2;

    mTimer.Start();
    Thread::SleepCurrentThread(1.0);
    result1 = mTimer.Stop();

    mTimer.Start();
    Thread::SleepCurrentThread(0.5);
    result2 = mTimer.Stop();

    ASSERT_GT(result1, 0.0);
    ASSERT_GT(result2, 0.0);
    ASSERT_GT(result1, result2);
}
