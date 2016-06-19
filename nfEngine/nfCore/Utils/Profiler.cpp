/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Profiler definitions
 */

#include "PCH.hpp"

#include "Profiler.hpp"


namespace NFE {
namespace Util {

unsigned int ProfilerNode::mCurrentStatsBuffer = 1;
unsigned int ProfilerNode::mOldStatsBuffer = 0;

ProfilerNode::ProfilerNode(const char* name)
    : mName(name)
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

void ProfilerNode::ClearAllStats()
{
    for (auto& stat : mStatsBuffer)
        stat.Reset();

    for (auto& child : mChildren)
        child->ClearAllStats();
}

void ProfilerNode::ClearCurrentStats()
{
    mStatsBuffer[mCurrentStatsBuffer] = ProfilerNodeStats();

    for (auto& child : mChildren)
        child->ClearCurrentStats();
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
        node->ClearAllStats();
}

void Profiler::SwitchAllStats()
{
    ProfilerNode::mOldStatsBuffer = ProfilerNode::mCurrentStatsBuffer;

    ++ProfilerNode::mCurrentStatsBuffer;
    if (ProfilerNode::mCurrentStatsBuffer >= NFE_PROFILER_STATSBUFFER_SIZE)
        ProfilerNode::mCurrentStatsBuffer = 0;

    for (auto& node : mNodes)
        node->ClearCurrentStats();
}

const ProfilerNodeArray& Profiler::GetNodes() const
{
    return mNodes;
}

} // namespace Util
} // namespace NFE
