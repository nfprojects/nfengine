/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Profiler definitions
 */

#include "PCH.hpp"

#include "Profiler.hpp"

namespace NFE {
namespace Util {


ProfilerNode::ProfilerNode(const char* name)
    : mName(name)
{
}

ProfilerNode::~ProfilerNode()
{
    for (const auto& node : mChildren)
        delete node;
}

ProfilerScope::ProfilerScope(const ProfilerNode* node)
    : mNode(node)
{
}

ProfilerScope::~ProfilerScope()
{
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

const ProfilerNode* Profiler::RegisterNode(const std::string& name, const ProfilerNode* parent)
{
    return RegisterNode(name.c_str(), parent);
}

const ProfilerNode* Profiler::RegisterNode(const char* name, const ProfilerNode* parent)
{
    ProfilerNode* newNode = new ProfilerNode(name);

    if (parent == nullptr)
        mNodes.push_back(newNode);
    else
        parent->mChildren.push_back(newNode);

    return newNode;
}


} // namespace Util
} // namespace NFE
