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
    friend class Profiler;

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


/**
 * Create a ProfilerScope object, which will perform profiling-related actions at its birth and
 * death.
 *
 * @param nodename Name of registered ProfilerNode object.
 */
#define PROFILER_SCOPE(nodename) ProfilerScope nodename##Scope(nodename)


/**
 * Register a root Profiler node.
 *
 * @param name     String containing name of the node. Used for display purposes.
 * @param nodename Name of created node pointer object. Used to refer in other Profiler functions.
 */
#define PROFILER_REGISTER_ROOT_NODE(name, nodename) \
        static const ProfilerNode* nodename = Profiler::Instance().RegisterNode(name, nullptr)

/**
 * Register a Profiler node, which is a child to other node.
 *
 * @param name     String containing name of the node. Used for display purposes.
 * @param nodename Name of created node pointer object. Used to refer in other Profiler functions.
 * @param parent   Parent node to which created child node will be attached.
 */
#define PROFILER_REGISTER_NODE(name, nodename, parent) \
        static const ProfilerNode* nodename = Profiler::Instance().RegisterNode(name, parent)

} // namespace Util
} // namespace NFE
