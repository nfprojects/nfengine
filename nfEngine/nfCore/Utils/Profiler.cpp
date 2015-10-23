/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Profiler definitions
 */

#include "PCH.hpp"

#include "Profiler.hpp"

namespace NFE {
namespace Util {

ProfilerNodeStats::ProfilerNodeStats()
    : time(0.0)
    , visitCount(0)
{
}


ProfilerNode::ProfilerNode(const char* name)
    : mName(name)
{
}

ProfilerNode::~ProfilerNode()
{
    for (const auto& node : mChildren)
        delete node;
}

void ProfilerNode::StartScope()
{
    mTimer.Start();
}

void ProfilerNode::StopScope()
{
    mStats.time = mTimer.Stop();
    mStats.visitCount++;
}

const ProfilerNodeStats& ProfilerNode::GetStats()
{
    return mStats;
}

void ProfilerNode::ResetStats()
{
    mStats = ProfilerNodeStats();

    for (auto& child : mChildren)
        child->ResetStats();
}


ProfilerScope::ProfilerScope(ProfilerNode* node)
    : mNode(node)
{
    mNode->StartScope();
}

ProfilerScope::~ProfilerScope()
{
    mNode->StopScope();
}


Profiler::Profiler()
{
}

Profiler::~Profiler()
{
    for (const auto& node : mNodes)
        delete node;
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
    ProfilerNode* newNode = new ProfilerNode(name);

    if (parent == nullptr)
        mNodes.push_back(newNode);
    else
        parent->mChildren.push_back(newNode);

    return newNode;
}

void Profiler::ResetAllStats()
{
    if (mNodes.empty())
        return;

    for (auto& node : mNodes)
        node->ResetStats();
}


} // namespace Util
} // namespace NFE
