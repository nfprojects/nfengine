#include "PCH.hpp"
#include "Utils/Profiler.hpp"

using namespace NFE::Util;

class ProfilerTest : public testing::Test
{
protected:

};

PROFILER_REGISTER_ROOT_NODE("Test", Test);
    PROFILER_REGISTER_NODE("Child1", Child1, Test);
        PROFILER_REGISTER_NODE("Child1Child", Child1Child, Child1);
    PROFILER_REGISTER_NODE("Child2", Child2, Test);
    PROFILER_REGISTER_NODE("Child3", Child3, Test);

TEST_F(ProfilerTest, Basic)
{
    ASSERT_NE(Test, nullptr);
    ASSERT_NE(Child1, nullptr);
    ASSERT_NE(Child1Child, nullptr);
    ASSERT_NE(Child2, nullptr);
    ASSERT_NE(Child3, nullptr);
}

TEST_F(ProfilerTest, Scope)
{
    {
        PROFILER_SCOPE(Child1Child);
    }
}
