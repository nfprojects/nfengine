/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Engine's config mangager definition
 */

#include "../PCH.hpp"

#include "ConfigManager.hpp"
#include "ConfigVariable.hpp"

#include "nfCommon/System/Assertion.hpp"
#include "nfCommon/Config/Config.hpp"
#include "nfCommon/Config/ConfigValue.hpp"
#include "nfCommon/FileSystem/File.hpp"
#include "nfCommon/Logger/Logger.hpp"

namespace NFE {

using namespace Common;

const String gMainConfigFilePath = "engineConfig.cfg";

ConfigManager::ConfigManager()
    : mNumVariables(0)
    , mConfig(new Common::Config)
{
}

ConfigManager& ConfigManager::GetInstance()
{
    static ConfigManager gConfigManager;
    return gConfigManager;
}

bool ConfigManager::Initialize()
{
    NFE_LOG_INFO("%u config variables registered", mNumVariables);

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

    if (node->variables.find(segments.back()) != node->variables.end())
    {
        NFE_LOG_ERROR("Variable '%s' declared twice", path);
    }

    // place variable in the target node
    node->variables[segments.back()] = variable;
    mNumVariables++;
}

bool ConfigManager::LoadConfiguration()
{
    NFE_LOG_INFO("Loading engine's configuration...");

    std::vector<char> configFileStr;
    size_t configFileSize = 0;

    // load config file to memory
    File file(gMainConfigFilePath, AccessMode::Read);
    configFileSize = static_cast<size_t>(file.GetSize());
    configFileStr.resize(configFileSize + 1);
    if (file.Read(configFileStr.data(), configFileSize) != configFileSize)
    {
        NFE_LOG_ERROR("Failed to read config file");
        return false;
    }
    configFileStr[configFileSize] = '\0';

    // parse
    if (!mConfig->Parse(configFileStr.data()))
    {
        NFE_LOG_ERROR("Failed to parse engine's config file");
        return false;
    }

    // find config variables
    if (!mRootNode.Parse(*mConfig, mConfig->GetRootNode()))
    {
        NFE_LOG_ERROR("Failed to load config variables");
        return false;
    }

    NFE_LOG_SUCCESS("Engine's configuration loaded");
    return true;
}

bool ConfigManager::Node::Parse(const Common::Config& config, ConfigObjectNodePtr node)
{
    auto callback = [&](StringView key, const ConfigValue& value)
    {
        // find matching subnodes (objects)
        for (auto& subNode : subNodes)
        {
            if (subNode.first.c_str() == key && value.IsObject())
            {
                subNode.second->Parse(config, value.GetObj());
            }
        }

        // find matching variables (values)
        for (auto& variable : variables)
        {
            if (variable.first.c_str() == key && !value.IsObject())
            {
                if (variable.second->ParseConfigValue(value))
                {
                    NFE_LOG_DEBUG("'%s' = %s", variable.second->GetPath(), variable.second->ToString().c_str());
                }
            }
        }

        return true;
    };

    config.Iterate(callback, node);
    return true;
}

} // namespace NFE
