/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

#define INVALID_NODE_PTR static_cast<uint32>(-1)

/// Predeclarations
class ConfigTokenizer;
struct Token;

/**
 * Pointers to config structure elements (values, array nodes and object nodes).
 * We access them via index instead of pointers, because:
 * 1. They take less space on 64-bits.
 * 2. Buffers containing these elements will be dynamically expanded during parsing and inserting,
 *    so the pointers can turn invalid in the middle of these processes.
 */
typedef uint32 ConfigObjectNodePtr;
typedef uint32 ConfigArrayNodePtr;
typedef uint32 ConfigValuePtr;


class ConfigObject
{
    friend class Config;
    friend class ConfigValue;

    ConfigObjectNodePtr mHead;
    ConfigObjectNodePtr mTail;

public:
    ConfigObject() : mHead(INVALID_NODE_PTR), mTail(INVALID_NODE_PTR) { }
};

class ConfigArray
{
    friend class Config;
    friend class ConfigValue;

    ConfigArrayNodePtr mHead;
    ConfigArrayNodePtr mTail;

public:
    ConfigArray() : mHead(INVALID_NODE_PTR), mTail(INVALID_NODE_PTR) { }
};

/**
 * Class representing a value in the config tree. A value can be of simple type
 * (bool, int, float, string), subobject or an array of values.
 */
class ConfigValue
{
    friend class Config;

    enum class Type
    {
        None, Bool, Int, Float, String, Object, Array,
    };

    union
    {
        bool boolData;
        int intData;
        float floatData;
        const char* stringData;
        ConfigObjectNodePtr object;
        ConfigArrayNodePtr array;
    };

    Type type;

public:
    ConfigValue() : type(Type::None), object(INVALID_NODE_PTR) { }

    /// Constructors
    ConfigValue(bool val) { type = Type::Bool; boolData = val; }
    ConfigValue(int val) { type = Type::Int; intData = val; }
    ConfigValue(float val) { type = Type::Float; floatData = val; }
    ConfigValue(const char* val) { type = Type::String; stringData = val; }
    ConfigValue(const ConfigObject& val) { type = Type::Object; object = val.mHead; }
    // TODO: array

    /// Checkers
    NFE_INLINE bool IsBool() const { return type == Type::Bool; }
    NFE_INLINE bool IsInt() const { return type == Type::Int; }
    NFE_INLINE bool IsFloat() const { return type == Type::Float; }
    NFE_INLINE bool IsString() const { return type == Type::String; }
    NFE_INLINE bool IsObject() const { return type == Type::Object; }
    NFE_INLINE bool IsArray() const { return type == Type::Array; }

    /// Getters
    NFE_INLINE bool GetBool() const { return boolData; }
    NFE_INLINE int GetInt() const { return intData; }
    NFE_INLINE float GetFloat() const { return floatData; }
    NFE_INLINE const char* GetString() const { return stringData; }
    NFE_INLINE ConfigObjectNodePtr GetObj() const { return object; }
    NFE_INLINE ConfigArrayNodePtr GetArray() const { return array; }
};

/**
 * Config array node. Used for fast insertions and iterations.
 */
struct ConfigArrayNode
{
    ConfigValuePtr valuePtr;
    ConfigArrayNodePtr next;

    ConfigArrayNode() : valuePtr(INVALID_NODE_PTR), next(INVALID_NODE_PTR) { }
};

/**
 * Config object node. Used for fast insertions and iterations.
 */
struct ConfigObjectNode
{
    const char* name;
    ConfigValuePtr valuePtr;
    ConfigObjectNodePtr next;

    ConfigObjectNode() : name(nullptr), valuePtr(INVALID_NODE_PTR), next(INVALID_NODE_PTR) { }
};

/**
 * The main class used for parsing and managing config structures.
 */
class NFCOMMON_API Config
{
    std::unique_ptr<char[]> mStringCopy;

    ConfigObjectNodePtr mRootNode;

    /// Buffers of the config elements:
    std::vector<ConfigValue> mValues;
    std::vector<ConfigObjectNode> mObjectNodes;
    std::vector<ConfigArrayNode> mArrayNodes;

    /// Methods for allocations of config elements:
    ConfigValuePtr AllocateValue();
    ConfigObjectNodePtr AllocateObjectNode();
    ConfigArrayNodePtr AllocateArrayNode();

    /// Parser methods:
    bool ParseValue(ConfigTokenizer& tokenizer, const Token& token, ConfigValue& value);
    bool ParseArray(ConfigTokenizer& tokenizer, ConfigValue& value);
    ConfigObjectNodePtr ParseObject(ConfigTokenizer& tokenizer);

    /// Generator methods (config to string conversion):
    void ValueToString(std::string& outputStr, ConfigValuePtr valuePtr) const;
    void ObjectToString(std::string& outputStr, ConfigObjectNodePtr objectPtr) const;

    Config(const Config&) = delete;
    Config& operator= (const Config&) = delete;

public:
    // callback functions called when walking a config object
    typedef std::function<void(const char* key, const ConfigValue& value)> ObjectIterator;

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
    NFE_INLINE ConfigObjectNodePtr GetRootNode() const
    {
        return mRootNode;
    }

    /**
     * Set new root
     */
    void SetRoot(const ConfigObject& rootObject);

    /**
     * Convert config to a string.
     * @note When modifying parsed config, the original formatting won't be preserved
     *       (intendation, comments, etc.).
     */
    std::string ToString() const;

    /**
     * Iterate through config object.
     * @param callback Callback function to be called for each object value.
     * @param node Starting node. Set to NULL if you want to interate root node.
     */
    void Iterate(const ObjectIterator& callback,
                 ConfigObjectNodePtr node = INVALID_NODE_PTR) const;

    /**
     * Add a value to a config object.
     * @param configObject Reference to helper ConfigObject object.
     * @param key          Key string.
     * @param val          Value.
     */
    Config& AddValue(ConfigObject& configObject, const char* key, const ConfigValue& val);

    /**
     * Add a value to a config array.
     * @param configArray Reference to helper ConfigArray object.
     * @param val         Value.
     */
    Config& AddValue(ConfigArray& configArray, const ConfigValue& val);
};

} // namespace Common
} // namespace NFE
