/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Profiler declarations
 */

#pragma once

#include "../Core.hpp"

#include "Engine/Common/System/Timer.hpp"

#include <memory>
#include <vector>


namespace NFE {
namespace Util {

class ProfilerNode;
typedef std::unique_ptr<ProfilerNode> ProfilerNodePtr;
typedef std::vector<ProfilerNodePtr> ProfilerNodeArray;

#define NFE_PROFILER_STATSBUFFER_SIZE 2

/**
 * Structure with statistics gathered by every node.
 */
struct ProfilerNodeStats
{
    double time;                ///< Time between StartScope and StopScope calls
    unsigned int visitCount;    ///< Amount of times the Node has been visited

    ProfilerNodeStats();
};

/**
 * Object representing single node to profile in code.
 *
 * @remarks This class should be created by Profiler singleton
 */
class CORE_API ProfilerNode final
{
    friend class Profiler;
    friend class ProfilerScope;

    const char* mName;
    ProfilerNodeArray mChildren;

    NFE::Common::Timer mTimer;
    ProfilerNodeStats mStatsBuffer[NFE_PROFILER_STATSBUFFER_SIZE];

    static unsigned int mCurrentStatsBuffer;
    static unsigned int mOldStatsBuffer;

public:
    ProfilerNode(const char* mName);
    ProfilerNode(const ProfilerNode&) = delete;
    ProfilerNode operator=(const ProfilerNode&) = delete;

    /**
     * Trigger scope-related measurements at the beginning of a scope.
     *
     * @remarks This function should be usually called by ProfilerScope object.
     */
    void StartScope();

    /**
     * Trigger scope-related measurements at the end of a scope.
     *
     * @remarks This function should be usually called by ProfilerScope object.
     */
    void StopScope();

    /**
     * Acquire statistics gathered so far.
     *
     * @return Structure containing statistics gathered by node.
     */
    const ProfilerNodeStats& GetStats() const;

    /**
     * Acquire node's name string.
     */
    const char* GetName() const;

    /**
     * Get reference to Node's children.
     */
    const ProfilerNodeArray& GetChildren() const;

    /**
     * Reset statistics gathered by the node.
     */
    void ClearAllStats();

    /**
     * Clear current stats buffer. Done performing a switch to prepare the buffer before use.
     */
    void ClearCurrentStats();
};

/**
 * Helper RAII object which triggers profiling-related statistic gathering
 *
 * @remarks Use PROFILER_SCOPE macro to easily set scopes in code
 */
class CORE_API ProfilerScope final
{
    ProfilerNode* mNode;

public:
    ProfilerScope(const ProfilerScope&) = delete;
    ProfilerScope& operator=(const ProfilerScope&) = delete;

    NFE_INLINE ProfilerScope(ProfilerNode* node)
        : mNode(node)
    {
        mNode->StartScope();
    }

    NFE_INLINE ~ProfilerScope()
    {
        mNode->StopScope();
    }
};


/**
 * Singleton used to gather and calculate profiler statistics from nodes.
 */
class CORE_API Profiler final
{
    Profiler();
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    ProfilerNodeArray mNodes;

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
    ProfilerNode* RegisterNode(const std::string& name, ProfilerNode* parent);

    /**
     * Register new Profiler node
     *
     * @param name   Human-readable node name
     * @param parent Pointer to parent node. Can be nullptr - then Profiler will register
     *               new root node.
     *
     * @return Pointer to newly registered node
     */
    ProfilerNode* RegisterNode(const char* name, ProfilerNode* parent);

    /**
     * Traverses the Node Tree and resets all the statistics to default.
     *
     * For default values see ProfilerNodeStats structure definition.
     */
    void ResetAllStats();

    /**
     * Switches stats buffer to a new one. The old stats are set to be accessible for reading.
     */
    void SwitchAllStats();

    /**
     * Returns reference for all root Nodes kept within Profiler instance.
     */
    const ProfilerNodeArray& GetNodes() const;
};


/**
 * Create a ProfilerScope object, which will perform profiling-related actions at its birth and
 * death.
 *
 * @param nodename Name of registered ProfilerNode object.
 */
#define PROFILER_SCOPE(nodename) NFE::Util::ProfilerScope nodename##Scope(nodename)


/**
 * Register a root Profiler node.
 *
 * @param name     String containing name of the node. Used for display purposes.
 * @param nodename Name of created node pointer object. Used to refer in other Profiler functions.
 */
#define PROFILER_REGISTER_ROOT_NODE(name, nodename) \
        NFE::Util::ProfilerNode* nodename = NFE::Util::Profiler::Instance().RegisterNode(name, nullptr)

/**
 * Register a Profiler node, which is a child to other node.
 *
 * @param name     String containing name of the node. Used for display purposes.
 * @param nodename Name of created node pointer object. Used to refer in other Profiler functions.
 * @param parent   Parent node to which created child node will be attached.
 */
#define PROFILER_REGISTER_NODE(name, nodename, parent) \
        NFE::Util::ProfilerNode* nodename = NFE::Util::Profiler::Instance().RegisterNode(name, parent)

/**
 * Declare a Profiler Node.
 *
 * @param nodename Name of node to be declared. Same name must be later on used in
 *                 PROFILER_REGISTER_NODE macros.
 *
 * This macro produces a node declaration, useful in header files where the Node must be visible
 * to other files in the project.
 */
#define PROFILER_DECLARE_NODE(nodename) \
        extern NFE::Util::ProfilerNode* nodename

} // namespace Util
} // namespace NFE
