/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

/// Predeclarations
class ConfigTokenizer;
struct Token;
struct ConfigNode;
struct ConfigArray;
struct ConfigObjectNode;
struct ConfigArrayNode;
template <class T> class DataTranslator;

class ConfigValue
{
    friend class ConfigParser;

    enum class Type
    {
        None,
        Bool,
        Int,
        Float,
        String,
        Object,
        Array,
    };

    union
    {
        bool boolData;
        int intData;
        float floatData;
        const char* stringData;
        ConfigObjectNode* object;
        ConfigArrayNode* array;
    };

    Type type;

public:
    ConfigValue() : type(Type::None) { }

    /// Constructors:
    ConfigValue(bool val) { type = Type::Bool; boolData = val; }
    ConfigValue(int val) { type = Type::Int; intData = val; }
    ConfigValue(float val) { type = Type::Float; floatData = val; }
    ConfigValue(const char* val) { type = Type::String; stringData = val; }
    // TODO: objects and arrays

    /// Checkers:
    NFE_INLINE bool IsBool() const { return type == Type::Bool; }
    NFE_INLINE bool IsInt() const { return type == Type::Int; }
    NFE_INLINE bool IsFloat() const { return type == Type::Float; }
    NFE_INLINE bool IsString() const { return type == Type::String; }
    NFE_INLINE bool IsObject() const { return type == Type::Object; }
    NFE_INLINE bool IsArray() const { return type == Type::Array; }

    template<typename T>
    T Get() const;
};

template<>
int ConfigValue::Get() const
{
    return intData;
}

template<>
bool ConfigValue::Get() const
{
    return boolData;
}

template<>
float ConfigValue::Get() const
{
    return floatData;
}

template<>
const char* ConfigValue::Get() const
{
    return stringData;
}

template<>
const ConfigObjectNode* ConfigValue::Get() const
{
    return object;
}

struct ConfigObject
{
    ConfigObjectNode* head;
    ConfigObjectNode* tail;
    size_t size;
};

struct ConfigArray
{
    ConfigArrayNode* head;
    ConfigArrayNode* tail;
    size_t size;
};

/**
 * Helper class - config array node.
 */
struct ConfigArrayNode
{
    ConfigValue value;
    ConfigArrayNode* next;

    ConfigArrayNode() : next(nullptr) { }
};

/**
 * Helper class - config object node.
 */
struct ConfigObjectNode
{
    const char* name;
    ConfigValue value;
    ConfigObjectNode* next;

    ConfigObjectNode();
};

/**
 * The main class used for parsing and managing config structures.
 */
class NFCOMMON_API ConfigParser
{
private:
    std::unique_ptr<char[]> mStringCopy;

    ConfigObjectNode* mRootNode;

    // TODO: temporary (memory pool or sth should be used)
    std::vector<std::unique_ptr<ConfigObjectNode>> mObjectNodes;
    std::vector<std::unique_ptr<ConfigArrayNode>> mArrayNodes;

    ConfigObjectNode* AllocateObjectNode();
    ConfigArrayNode* AllocateArrayNode();

    bool ParseValue(ConfigTokenizer& tokenizer, const Token& token, ConfigValue& value);
    bool ParseArray(ConfigTokenizer& tokenizer, ConfigValue& value);
    ConfigObjectNode* ParseObject(ConfigTokenizer& tokenizer);

    ConfigParser(const ConfigParser&) = delete;
    ConfigParser& operator= (const ConfigParser&) = delete;

public:
    typedef std::function<void(const char* key, const ConfigValue& value)> InteratorCallback;

    ConfigParser();

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
    NFE_INLINE ConfigObjectNode* GetRootNode() const
    {
        return mRootNode;
    }

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
    void Iterate(const InteratorCallback& callback, const ConfigObjectNode* node = nullptr) const;

    /**
     * Preallocates config object.
     */
    ConfigObject* CreateObject();

    /**
     * Create array value.
     * @param values
     * @param numValues
     * @return
     */
    ConfigArray* CreateArray(const ConfigValue* values, size_t numValues);

    /**
     * Add a key-value pair to an existing object.
     * @return True on success.
     */
    bool AddToObject(ConfigObject* object, const char* keyName, const ConfigValue& value);

    void DebugPrintValue(const ConfigValue& value, int indent = 0) const;
    void DebugPrint(const ConfigObjectNode* node, int indent = 0) const;

    template <class T>
    bool TranslateConfigObject(const ConfigObjectNode* node,
                               DataTranslator<T>& translator,
                               T& object) const;
};

} // namespace Common
} // namespace NFE
