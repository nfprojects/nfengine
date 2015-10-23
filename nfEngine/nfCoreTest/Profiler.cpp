#include "PCH.hpp"
#include "Utils/Profiler.hpp"

using namespace NFE::Util;

class ProfilerTest : public testing::Test
{
protected:

};

PROFILER_REGISTER_ROOT_NODE(Test);
PROFILER_REGISTER_NODE(Child, Test);

TEST_F(ProfilerTest, Basic)
{
    ASSERT_NE(TestNode, nullptr);
    ASSERT_NE(ChildNode, nullptr);
}

TEST_F(ProfilerTest, Scope)
{

}

