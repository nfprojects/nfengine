/**
 * @file
 * @author Witek902
 * @brief  Config parser declaration.
 */

#pragma once

#include "ConfigInterface.hpp"
#include "../Containers/DynArray.hpp"
#include "../Containers/UniquePtr.hpp"
#include "../Containers/String.hpp"

#include <memory>
#include <functional>

namespace NFE {
namespace Common {

/**
 * The main class used for parsing and managing config structures.
 */
class NFCOMMON_API Config : public IConfig
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
    void ValueToString(String& out, ConfigValuePtr valuePtr, int indent) const;
    void ObjectToString(String& out, ConfigObjectNodePtr objectPtr, int indent) const;

    Config(const Config&) = delete;
    Config& operator= (const Config&) = delete;

public:
    // callback functions called when walking a config object and array
    typedef std::function<bool(StringView key, const ConfigValue& value)> ObjectIterator;
    typedef std::function<bool(uint32 index, const ConfigValue& value)> ArrayIterator;

    Config();

    virtual void Clear() override;

    virtual bool ParseInPlace(char* string) override;

    virtual bool Parse(const StringView& string) override;

    virtual ConfigObjectNodePtr GetRootNode() const override
    {
        return mRootNode;
    }

    virtual void SetRoot(const ConfigObject& rootObject) override;

    virtual String ToString(bool format = true) const override;

    virtual void Iterate(const ObjectIterator& callback, ConfigObjectNodePtr node = INVALID_NODE_PTR) const override;
    virtual void IterateArray(const ArrayIterator& callback, ConfigArrayNodePtr node) const override;

    virtual Config& AddValue(ConfigObject& configObject, StringView key, const ConfigValue& val) override;
    virtual Config& AddValue(ConfigArray& configArray, const ConfigValue& val) override;

    template <class T>
    bool TranslateConfigObject(ConfigObjectNodePtr node,
                               DataTranslator<T>& translator,
                               T& object) const;
};


} // namespace Common
} // namespace NFE
