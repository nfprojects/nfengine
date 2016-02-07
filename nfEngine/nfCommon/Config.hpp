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

struct NFCOMMON_API ConfigValue
{
    enum class Type
    {
        None,
        Integer,
        Bool,
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
};

struct NFCOMMON_API ConfigArray
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
    ConfigParser();
    bool Parse(const char* string);
    bool ParseInPlace(char* string);

    NFE_INLINE ConfigNode* GetRootNode() const
    {
        return mRootNode;
    }

    void DebugPrintValue(const ConfigValue& value, int indent = 0) const;
    void DebugPrint(ConfigNode* node, int indent = 0) const;
};

} // namespace Common
} // namespace NFE
