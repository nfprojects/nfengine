/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Engine's config mangager definition
 */

#include "../PCH.hpp"

#include "ConfigManager.hpp"
#include "ConfigVariable.hpp"

#include "nfCommon/Config.hpp"
#include "nfCommon/File.hpp"
#include "nfCommon/Logger.hpp"

namespace NFE {

using namespace Common;

const char* MAIN_CONFIG_FILE_PATH = "engineConfig.cfg";

ConfigManager::ConfigManager()
    : mNumVariables(0)
{
}

ConfigManager& ConfigManager::GetInstance()
{
    static ConfigManager gConfigManager;
    return gConfigManager;
}

bool ConfigManager::Initialize()
{
    LOG_INFO("%u config variables registered", mNumVariables);

    // TODO: load config from multiple config files
    return LoadConfiguration();
}

void ConfigManager::RegisterVariable(IConfigVariable* variable)
{
    const char* path = variable->GetPath();

    std::stringstream sstream(path);
    std::string segment;
    std::vector<std::string> segments;
    while (std::getline(sstream, segment, '/'))
    {
        segments.push_back(segment);
    }

    size_t numSegments = segments.size();
    NFE_ASSERT(numSegments > 0, "Invalid config variable path");

    // find/create node for the variable
    Node* node = &mRootNode;
    if (numSegments > 1)
    {
        for (size_t i = 0; i < numSegments - 1; ++i)
        {
            const auto it = node->subNodes.find(segments[i]);
            if (it == node->subNodes.end())
            {
                // create new node
                NodePtr newNode(new Node);
                Node* nextNode = newNode.get();
                node->subNodes[segments[i]] = std::move(newNode);
                node = nextNode;
            }
            else
            {
                // pass existing node
                node = it->second.get();
            }
        }
    }

    if (node->variables.find(segments[numSegments - 1]) != node->variables.end())
    {
        LOG_ERROR("Variable '%s' declared twice", path);
    }

    // place variable in the target node
    node->variables[segments[numSegments - 1]] = variable;
    mNumVariables++;
}

bool ConfigManager::LoadConfiguration()
{
    LOG_INFO("Loading engine's configuration...");

    std::vector<char> configFileStr;
    size_t configFileSize = 0;

    // load config file to memory
    File file(MAIN_CONFIG_FILE_PATH, AccessMode::Read);
    configFileSize = static_cast<size_t>(file.GetSize());
    configFileStr.resize(configFileSize + 1);
    if (file.Read(configFileStr.data(), configFileSize) != configFileSize)
    {
        LOG_ERROR("Failed to read config file");
        return false;
    }
    configFileStr[configFileSize] = '\0';

    // parse
    Common::Config config;
    if (!config.Parse(configFileStr.data()))
    {
        LOG_ERROR("Failed to parse engine's config file");
        return false;
    }

    // find config variables
    if (!mRootNode.Parse(config, config.GetRootNode()))
    {
        LOG_ERROR("Failed to load config variables");
        return false;
    }

    LOG_SUCCESS("Engine's configuration loaded");
    return true;
}

bool ConfigManager::Node::Parse(const Common::Config& config, ConfigObjectNodePtr node)
{
    auto callback = [&](const char* key, const ConfigValue& value)
    {
        // find matching subnodes (objects)
        for (auto& subNode : subNodes)
        {
            if (subNode.first == key && value.IsObject())
            {
                subNode.second->Parse(config, value.GetObj());
            }
        }

        // find matching variables (values)
        for (auto& variable : variables)
        {
            if (variable.first == key && !value.IsObject())
            {
                if (variable.second->ParseConfigValue(value))
                {
                    LOG_DEBUG("'%s' = %s", variable.second->GetPath(), variable.second->ToString().c_str());
                }
            }
        }

        return true;
    };

    config.Iterate(callback, node);
    return true;
}

} // namespace NFE
