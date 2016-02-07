/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

class ConfigTokenizer;
struct Token;
struct ConfigNode;
struct ConfigArray;
template <class T> class DataTranslator;

struct ConfigValue
{
    enum class Type
    {
        None,
        Bool,
        Integer,
        Float,
        String,
        Object,
        Array,
    };

    union
    {
        bool booleanData;
        int integerData;
        float floatData;
        const char* stringData;
        ConfigNode* object;
        ConfigArray* array;
    };

    Type type;

    NFE_INLINE bool IsBool() const
    {
        return type == Type::Bool;
    }

    NFE_INLINE bool IsInt() const
    {
        return type == Type::Integer;
    }

    NFE_INLINE bool IsFloat() const
    {
        return type == Type::Float;
    }

    NFE_INLINE bool IsString() const
    {
        return type == Type::String;
    }

    NFE_INLINE bool IsObject() const
    {
        return type == Type::Object;
    }

    NFE_INLINE bool IsArray() const
    {
        return type == Type::Array;
    }

    template<typename T>
    T Get() const;
};

template<>
int ConfigValue::Get() const
{
    return integerData;
}

template<>
bool ConfigValue::Get() const
{
    return booleanData;
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
const ConfigNode* ConfigValue::Get() const
{
    return object;
}


struct ConfigArray
{
    ConfigValue value;
    ConfigArray* next;

    ConfigArray() : next(nullptr) { }
};

struct ConfigNode
{
    const char* name;
    ConfigValue value;
    ConfigNode* next;

    ConfigNode();
};

class NFCOMMON_API ConfigParser
{
private:
    std::unique_ptr<char[]> mStringCopy;

    ConfigNode* mRootNode;

    // TODO: temporary
    std::vector<std::unique_ptr<ConfigNode>> mNodes;
    std::vector<std::unique_ptr<ConfigArray>> mArrayNodes;

    ConfigNode* AllocateObjectNode();
    ConfigArray* AllocateArrayNode();

    bool ParseValue(ConfigTokenizer& tokenizer, const Token& token, ConfigValue& value);
    bool ParseArray(ConfigTokenizer& tokenizer, ConfigValue& value);
    ConfigNode* ParseObject(ConfigTokenizer& tokenizer);

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

    NFE_INLINE ConfigNode* GetRootNode() const
    {
        return mRootNode;
    }

    /**
     * Iterate through config object.
     * @param callback Callback function to be called for each object value.
     * @param node Starting node. Set to NULL if you want to interate root node.
     */
    void Iterate(const InteratorCallback& callback, const ConfigNode* node = nullptr) const;

    void DebugPrintValue(const ConfigValue& value, int indent = 0) const;
    void DebugPrint(const ConfigNode* node, int indent = 0) const;

    template <class T>
    bool TranslateConfigObject(const ConfigNode* node,
                               DataTranslator<T>& translator,
                               T& object) const;
};

} // namespace Common
} // namespace NFE
