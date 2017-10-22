/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once

#include "ConfigCommon.hpp"
#include "ConfigValue.hpp"
#include "../Containers/DynArray.hpp"
#include "../Containers/UniquePtr.hpp"

#include <memory>
#include <functional>
#include <vector>


namespace NFE {
namespace Common {

/**
 * The main class used for parsing and managing config structures.
 */
class NFCOMMON_API Config
{
    UniquePtr<char[]> mStringCopy;

    ConfigObjectNodePtr mRootNode;

    /// Buffers of the config elements:
    DynArray<ConfigValue> mValues;
    DynArray<ConfigObjectNode> mObjectNodes;
    DynArray<ConfigArrayNode> mArrayNodes;

    /// Methods for allocations of config elements:
    ConfigValuePtr AllocateValue();
    ConfigObjectNodePtr AllocateObjectNode();
    ConfigArrayNodePtr AllocateArrayNode();

    /// Parser methods:
    bool ParseValue(ConfigTokenizer& tokenizer, const Token& token, ConfigValue& value);
    bool ParseArray(ConfigTokenizer& tokenizer, ConfigValue& value);
    ConfigObjectNodePtr ParseObject(ConfigTokenizer& tokenizer);

    /// Generator methods (config to string conversion):
    void ValueToString(std::stringstream& out, ConfigValuePtr valuePtr, int indent) const;
    void ObjectToString(std::stringstream& out, ConfigObjectNodePtr objectPtr, int indent) const;

    Config(const Config&) = delete;
    Config& operator= (const Config&) = delete;

public:
    // callback functions called when walking a config object and array
    typedef std::function<bool(const char* key, const ConfigValue& value)> ObjectIterator;
    typedef std::function<bool(uint32 index, const ConfigValue& value)> ArrayIterator;

    Config();

    /**
     * Remove all data from the config tree and release resources.
     */
    void Clear();

    /**
     * Parse config located in read-write string.
     * @return True on success.
     * @note Provided string will be modified.
     */
    bool ParseInPlace(char* string);

    /**
     * Parse config located in read-only string.
     * @return True on success.
     * @note This function is a little bit slower than @p ParseInPlace, because it
     *       has to create local copy.
     */
    bool Parse(const char* string);

    /**
     * Get root object node.
     */
    ConfigObjectNodePtr GetRootNode() const
    {
        return mRootNode;
    }

    /**
     * Set new root
     */
    void SetRoot(const ConfigObject& rootObject);

    /**
     * Convert config to a string.
     * @param format Enable human-readable formatting.
     * @note When modifying parsed config, the original formatting won't be preserved
     *       (intendation, comments, etc.).
     * @return Config string.
     */
    std::string ToString(bool format = true) const;

    /**
     * Iterate through a config object.
     * @param callback Callback function to be called for each object value.
     * @param node     Starting node. Set to NULL if you want to iterate root node.
     */
    void Iterate(const ObjectIterator& callback,
                 ConfigObjectNodePtr node = INVALID_NODE_PTR) const;

    /**
     * Iterate through a config array.
     * @param callback Callback function to be called for each array value.
     * @param node     Starting node.
     */
    void IterateArray(const ArrayIterator& callback, ConfigArrayNodePtr node) const;

    /**
     * Add a value to a config object.
     * @param configObject Reference to helper ConfigObject object.
     * @param key          Key string.
     * @param val          Value.
     * @remarks The function returns a reference to "this" so writing AddValue chains is possible.
     */
    Config& AddValue(ConfigObject& configObject, const char* key, const ConfigValue& val);

    /**
     * Add a value to a config array.
     * @param configArray Reference to helper ConfigArray object.
     * @param val         Value.
     */
    Config& AddValue(ConfigArray& configArray, const ConfigValue& val);

    template <class T>
    bool TranslateConfigObject(ConfigObjectNodePtr node,
                               DataTranslator<T>& translator,
                               T& object) const;
};


} // namespace Common
} // namespace NFE
