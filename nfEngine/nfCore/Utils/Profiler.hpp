/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Profiler declarations
 */

#pragma once

#include "Core.hpp"

namespace NFE {
namespace Util {

// Predeclarations
class Profiler;

/**
 * Object representing single node to profile in code.
 *
 * @remarks This class should be created by Profiler singleton
 */
class ProfilerNode final
{
public:
    friend class Profiler;

private:
    ProfilerNode(const char* mName);

    const char* mName;
    mutable std::vector<ProfilerNode*> mChildren;

    // TODO all stats data for node go here
};

/**
 * Helper RAII object which triggers profiling-related statistic gathering
 *
 * @remarks Use PROFILER_SCOPE macro to easily set scopes in code
 */
class ProfilerScope final
{
public:
    ProfilerScope(const ProfilerNode* node);
    ~ProfilerScope();

private:
    const ProfilerNode* mNode;
};


/**
 * Singleton used to gather and calculate profiler statistics from nodes.
 */
class CORE_API Profiler final
{
public:
    /**
     * Acquires Profiler instance
     *
     * @return Profiler instance
     */
    static Profiler& Instance();

    /**
     * Register new Profiler node
     *
     * @param name   Human-readable node name
     * @param parent Pointer to parent node. Can be nullptr - then Profiler will register
     *               new root node.
     *
     * @return Pointer to newly registered node
     */
    const ProfilerNode* RegisterNode(const std::string& name, const ProfilerNode* parent);

    /**
     * Register new Profiler node
     *
     * @param name   Human-readable node name
     * @param parent Pointer to parent node. Can be nullptr - then Profiler will register
     *               new root node.
     *
     * @return Pointer to newly registered node
     */
    const ProfilerNode* RegisterNode(const char* name, const ProfilerNode* parent);

private:
    Profiler();
    ~Profiler();
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    std::vector<ProfilerNode*> mNodes;
};


#define PROFILER_SCOPE(x) ProfilerScope x##Scope(x)

#define PROFILER_REGISTER_ROOT_NODE(name) \
        static const ProfilerNode* name##Node = Profiler::Instance().RegisterNode(#name, nullptr)

#define PROFILER_REGISTER_NODE(name, parent) \
        static const ProfilerNode* name##Node = Profiler::Instance().RegisterNode(#name, parent##Node)

} // namespace Util
} // namespace NFE
