/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Profiler definitions
 */

#include "PCH.hpp"

#include "Profiler.hpp"

namespace {

const unsigned int STATS_BUFFER_SIZE = 2;

} // namespace

namespace NFE {
namespace Util {

ProfilerNodeStats::ProfilerNodeStats()
    : time(0.0)
    , visitCount(0)
{
}


ProfilerNode::ProfilerNode(const char* name)
    : mName(name)
    , mStatsBuffer(STATS_BUFFER_SIZE)
    , mCurrentStatsBuffer(0)
{
}

void ProfilerNode::StartScope()
{
    mTimer.Start();
}

void ProfilerNode::StopScope()
{
    mStatsBuffer[mCurrentStatsBuffer].time += mTimer.Stop();
    mStatsBuffer[mCurrentStatsBuffer].visitCount++;
}

const ProfilerNodeStats& ProfilerNode::GetStats() const
{
    return mStatsBuffer[mOldStatsBuffer];
}

const char* ProfilerNode::GetName() const
{
    return mName;
}

const ProfilerNodeArray& ProfilerNode::GetChildren() const
{
    return mChildren;
}

void ProfilerNode::ResetStats()
{
    for (auto& stat : mStatsBuffer)
        stat = ProfilerNodeStats();

    for (auto& child : mChildren)
        child->ResetStats();
}

void ProfilerNode::SwitchStats()
{
    mOldStatsBuffer = mCurrentStatsBuffer;

    ++mCurrentStatsBuffer;
    if (mCurrentStatsBuffer >= STATS_BUFFER_SIZE)
        mCurrentStatsBuffer = 0;

    mStatsBuffer[mCurrentStatsBuffer] = ProfilerNodeStats();

    for (auto& child : mChildren)
        child->SwitchStats();
}


Profiler::Profiler()
{
}

Profiler& Profiler::Instance()
{
    static Profiler instance;
    return instance;
}

ProfilerNode* Profiler::RegisterNode(const std::string& name, ProfilerNode* parent)
{
    return RegisterNode(name.c_str(), parent);
}

ProfilerNode* Profiler::RegisterNode(const char* name, ProfilerNode* parent)
{
    if (parent == nullptr)
    {
        mNodes.push_back(ProfilerNodePtr(new ProfilerNode(name)));
        return mNodes.back().get();
    }
    else
    {
        parent->mChildren.push_back(ProfilerNodePtr(new ProfilerNode(name)));
        return parent->mChildren.back().get();
    }
}

void Profiler::ResetAllStats()
{
    if (mNodes.empty())
        return;

    for (auto& node : mNodes)
        node->ResetStats();
}

void Profiler::SwitchAllStats()
{
    for (auto& node : mNodes)
        node->SwitchStats();
}

const ProfilerNodeArray& Profiler::GetNodes() const
{
    return mNodes;
}

} // namespace Util
} // namespace NFE
