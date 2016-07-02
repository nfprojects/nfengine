#include "PCH.hpp"
#include "nfCore/Utils/Profiler.hpp"

#include <thread>
#include <chrono>


using namespace NFE::Util;

class ProfilerTest : public testing::Test
{
protected:

};

namespace {
const int WAIT_TIME_ROOT = 100; // ms
const int WAIT_TIME_CHILD1 = 50; // ms
const int WAIT_TIME_CHILD1CHILD = 150; // ms
const int WAIT_TIME_CHILD2 = 20; // ms
const int WAIT_TIME_CHILD3 = 30; // ms
const int VISIT_COUNT_CHILD3 = 3;

const int WAIT_TIME_SUM = WAIT_TIME_ROOT + WAIT_TIME_CHILD1 + WAIT_TIME_CHILD1CHILD
                        + WAIT_TIME_CHILD2 + WAIT_TIME_CHILD3 * VISIT_COUNT_CHILD3;
} // namespace

PROFILER_REGISTER_ROOT_NODE("Root", Root);
    PROFILER_REGISTER_NODE("Child1", Child1, Root);
        PROFILER_REGISTER_NODE("Child1Child", Child1Child, Child1);
    PROFILER_REGISTER_NODE("Child2", Child2, Root);
    PROFILER_REGISTER_NODE("Child3", Child3, Root);

TEST_F(ProfilerTest, Basic)
{
    ASSERT_NE(Root, nullptr);
    ASSERT_NE(Child1, nullptr);
    ASSERT_NE(Child1Child, nullptr);
    ASSERT_NE(Child2, nullptr);
    ASSERT_NE(Child3, nullptr);
}

TEST_F(ProfilerTest, Scope)
{
    Profiler::Instance().ResetAllStats();

    {
        PROFILER_SCOPE(Child1Child);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_CHILD1CHILD));
    }

    Profiler::Instance().SwitchAllStats();

    const ProfilerNodeStats& stats = Child1Child->GetStats();
    ASSERT_NEAR(stats.time, static_cast<double>(WAIT_TIME_CHILD1CHILD) / 1000.0, 0.002);
    ASSERT_EQ(stats.visitCount, 1);
}

TEST_F(ProfilerTest, Stats)
{
    Profiler::Instance().ResetAllStats();

    {
        PROFILER_SCOPE(Root);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_ROOT));

        {
            PROFILER_SCOPE(Child1);
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_CHILD1));

            {
                PROFILER_SCOPE(Child1Child);
                std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_CHILD1CHILD));
            }
        }

        {
            PROFILER_SCOPE(Child2);
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_CHILD2));
        }

        for (int i = 0; i < VISIT_COUNT_CHILD3; ++i)
        {
            PROFILER_SCOPE(Child3);
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_CHILD3));
        }
    }

    Profiler::Instance().SwitchAllStats();

    // expected time is bigger here due to all the child nodes creating extra latency
    const ProfilerNodeStats& rootStats = Root->GetStats();
    EXPECT_NEAR(rootStats.time, static_cast<double>(WAIT_TIME_SUM) / 1000.0, 0.05);
    EXPECT_EQ(rootStats.visitCount, 1);

    const ProfilerNodeStats& child1Stats = Child1->GetStats();
    EXPECT_NEAR(child1Stats.time,
                static_cast<double>(WAIT_TIME_CHILD1 + WAIT_TIME_CHILD1CHILD) / 1000.0, 0.004);
    EXPECT_EQ(child1Stats.visitCount, 1);

    const ProfilerNodeStats& child1childStats = Child1Child->GetStats();
    EXPECT_NEAR(child1childStats.time, static_cast<double>(WAIT_TIME_CHILD1CHILD) / 1000.0, 0.004);
    EXPECT_EQ(child1childStats.visitCount, 1);

    const ProfilerNodeStats& child2Stats = Child2->GetStats();
    EXPECT_NEAR(child2Stats.time, static_cast<double>(WAIT_TIME_CHILD2) / 1000.0, 0.004);
    EXPECT_EQ(child2Stats.visitCount, 1);

    const ProfilerNodeStats& child3Stats = Child3->GetStats();
    EXPECT_NEAR(child3Stats.time, static_cast<double>(WAIT_TIME_CHILD3 * VISIT_COUNT_CHILD3) / 1000.0, 0.008);
    EXPECT_EQ(child3Stats.visitCount, VISIT_COUNT_CHILD3);
}

TEST_F(ProfilerTest, Buffers)
{
    Profiler::Instance().ResetAllStats();

    // gather statistics from "first frame"
    {
        PROFILER_SCOPE(Child1Child);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_CHILD1CHILD));
    }

    Profiler::Instance().SwitchAllStats();

    const ProfilerNodeStats& stats1 = Child1Child->GetStats();
    ASSERT_NEAR(stats1.time, static_cast<double>(WAIT_TIME_CHILD1CHILD) / 1000.0, 0.002);
    ASSERT_EQ(stats1.visitCount, 1);

    // gather statistics from "second frame"
    {
        PROFILER_SCOPE(Child1Child);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_CHILD1CHILD));
    }

    Profiler::Instance().SwitchAllStats();

    const ProfilerNodeStats& stats2 = Child1Child->GetStats();
    ASSERT_NEAR(stats2.time, static_cast<double>(WAIT_TIME_CHILD1CHILD) / 1000.0, 0.002);
    ASSERT_EQ(stats2.visitCount, 1);

    // switching should zero the data on the first buffer
    ASSERT_EQ(stats1.time, 0.0);
    ASSERT_EQ(stats1.visitCount, 0);
}
