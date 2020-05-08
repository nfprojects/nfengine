/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once

#include "ConfigCommon.hpp"
#include "ConfigValue.hpp"

#include <memory>
#include <functional>


namespace NFE {
namespace Common {

/**
 * The main class used for parsing and managing config structures.
 */
class NFCOMMON_API IConfig
{
public:
    // callback functions called when walking a config object and array
    typedef std::function<bool(StringView key, const ConfigValue& value)> ObjectIterator;
    typedef std::function<bool(uint32 index, const ConfigValue& value)> ArrayIterator;

    virtual ~IConfig() { }

    /**
     * Remove all data from the config tree and release resources.
     */
    virtual void Clear() = 0;

    /**
     * Parse config located in read-write string.
     * @return True on success.
     * @note Provided string will be modified.
     */
    virtual bool ParseInPlace(char* string) = 0;

    /**
     * Parse config located in read-only string.
     * @return True on success.
     * @note This function is a little bit slower than @p ParseInPlace, because it
     *       has to create local copy.
     */
    virtual bool Parse(const char* string) = 0;
    
    /**
     * Get root object node.
     */
    virtual ConfigObjectNodePtr GetRootNode() const = 0;

    /**
     * Set new root
     */
    virtual void SetRoot(const ConfigObject& rootObject) = 0;

    /**
     * Convert config to a string.
     * @param format Enable human-readable formatting.
     * @note When modifying parsed config, the original formatting won't be preserved
     *       (intendation, comments, etc.).
     * @return Config string.
     */
    virtual String ToString(bool format = true) const = 0;

    /**
     * Iterate through a config object.
     * @param callback Callback function to be called for each object value.
     * @param node     Starting node. Set to NULL if you want to iterate root node.
     */
    virtual void Iterate(const ObjectIterator& callback,
                 ConfigObjectNodePtr node = INVALID_NODE_PTR) const = 0;

    /**
     * Iterate through a config array.
     * @param callback Callback function to be called for each array value.
     * @param node     Starting node.
     */
    virtual void IterateArray(const ArrayIterator& callback, ConfigArrayNodePtr node) const = 0;

    /**
     * Add a value to a config object.
     * @param configObject Reference to helper ConfigObject object.
     * @param key          Key string.
     * @param val          Value.
     * @remarks The function returns a reference to "this" so writing AddValue chains is possible.
     */
    virtual Config& AddValue(ConfigObject& configObject, StringView key, const ConfigValue& val) = 0;

    /**
     * Add a value to a config array.
     * @param configArray Reference to helper ConfigArray object.
     * @param val         Value.
     */
    virtual Config& AddValue(ConfigArray& configArray, const ConfigValue& val) = 0;
};


} // namespace Common
} // namespace NFE
