/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser declaration.
 */

#pragma once
#include "nfCommon.hpp"

namespace NFE {
namespace Common {

class NFCOMMON_API ConfigNode
{
    friend class ConfigParser;

    enum class Type
    {
        None,
        Integer,
        Float,
        String,
        Object,
        Array
    };

    const char* mName;

    union
    {
        bool booleanData;
        int integerData;
        float floatData;
        const char* stringData;
        ConfigNode* nodeData;
    };

    Type mType;

public:
    ConfigNode();
};


class NFCOMMON_API ConfigParser
{
private:
    std::unique_ptr<char[]> mStringCopy;

    ConfigNode* mRootNode;

    // TODO: temporary
    std::vector<std::unique_ptr<ConfigNode>> mNodes;

    ConfigNode* ParseObject(const char* string);

    ConfigParser(const ConfigParser&) = delete;
    ConfigParser& operator= (const ConfigParser&) = delete;

public:
    ConfigParser();

    ConfigNode* AllocateNode();

    bool Parse(const char* string);
    bool ParseInPlace(char* string);
};

} // namespace Common
} // namespace NFE
