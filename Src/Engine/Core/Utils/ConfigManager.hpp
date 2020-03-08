/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Engine's config manager declaration
 */

#pragma once

#include "../Core.hpp"
#include "../../Common/nfCommon.hpp"


namespace NFE {

// predeclarations
namespace Common {
class Config;
class ConfigValue;
}

class IConfigVariable;

/**
 * Engine's configuration manager.
 */
class ConfigManager
{
    struct Node;
    using NodePtr = std::unique_ptr<Node>;
    using SubNodesMap = std::map<std::string, NodePtr>;
    using VariablesMap = std::map<std::string, IConfigVariable*>;

    struct Node
    {
        SubNodesMap subNodes;
        VariablesMap variables;

        /**
         * Iterate through nfCommon's config and find matching variables.
         */
        bool Parse(const Common::Config& config, uint32 node);
    };

    NFE_MAKE_NONCOPYABLE(ConfigManager)
    NFE_MAKE_NONMOVEABLE(ConfigManager)

    // make the class a singleton
    ConfigManager();

    Node mRootNode;
    uint32 mNumVariables;

    // we need to keep the config all the time, because there are the strings located
    std::unique_ptr<Common::Config> mConfig;

public:
    static ConfigManager& GetInstance();

    void RegisterVariable(IConfigVariable* variable);

    bool LoadConfiguration();

    bool Initialize();
};

} // namespace NFE
