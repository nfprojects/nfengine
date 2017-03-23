/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser definition.
 */

#include "PCH.hpp"
#include "Config.hpp"
#include "ConfigTokenizer.hpp"
#include "Logger/Logger.hpp"

#include <sstream>
#include <string.h>


namespace NFE {
namespace Common {

Config::Config()
    : mRootNode(INVALID_NODE_PTR)
{
}

void Config::Clear()
{
    mRootNode = INVALID_NODE_PTR;
    mValues.clear();
    mObjectNodes.clear();
    mArrayNodes.clear();
}

void Config::SetRoot(const ConfigObject& rootObject)
{
    mRootNode = rootObject.mHead;
}

ConfigValuePtr Config::AllocateValue()
{
    ConfigValuePtr ptr = static_cast<ConfigValuePtr>(mValues.size());
    mValues.push_back(ConfigValue());
    return ptr;
}

ConfigObjectNodePtr Config::AllocateObjectNode()
{
    ConfigObjectNodePtr nodePtr = static_cast<ConfigObjectNodePtr>(mObjectNodes.size());
    mObjectNodes.push_back(ConfigObjectNode());
    return nodePtr;
}

ConfigArrayNodePtr Config::AllocateArrayNode()
{
    ConfigArrayNodePtr nodePtr = static_cast<ConfigArrayNodePtr>(mArrayNodes.size());
    mArrayNodes.push_back(ConfigArrayNode());
    return nodePtr;
}

Config& Config::AddValue(ConfigObject& configObject, const char* key, const ConfigValue& val)
{
    ConfigObjectNodePtr node = AllocateObjectNode();

    // update linked list
    if (configObject.mHead == INVALID_NODE_PTR)
        configObject.mHead = node;
    else
        mObjectNodes[configObject.mTail].next = node;
    configObject.mTail = node;

    ConfigValuePtr valuePtr = AllocateValue();
    mValues[valuePtr] = val;
    mObjectNodes[node].name = key;
    mObjectNodes[node].valuePtr = valuePtr;

    return *this;
}

Config& Config::AddValue(ConfigArray& configArray, const ConfigValue& val)
{
    ConfigArrayNodePtr node = AllocateArrayNode();

    // update linked list
    if (configArray.mHead == INVALID_NODE_PTR)
        configArray.mHead = node;
    else
        mArrayNodes[configArray.mTail].next = node;
    configArray.mTail = node;

    ConfigValuePtr valuePtr = AllocateValue();
    mValues[valuePtr] = val;
    mArrayNodes[node].valuePtr = valuePtr;

    return *this;
}

bool Config::Parse(const char* string)
{
    size_t strLength = strlen(string) + 1;
    mStringCopy.reset(new char[strLength]);
    memcpy(mStringCopy.get(), string, strLength);
    return ParseInPlace(mStringCopy.get());
}

bool Config::ParseValue(ConfigTokenizer& tokenizer, const Token& token, ConfigValue& value)
{
    switch (token.type)
    {
    case Token::Type::Integer:
        value.type = ConfigValue::Type::Int;
        value.intData = token.integerData;
        break;
    case Token::Type::Float:
        value.type = ConfigValue::Type::Float;
        value.floatData = token.floatData;
        break;
    case Token::Type::Bool:
        value.type = ConfigValue::Type::Bool;
        value.boolData = token.booleanData;
        break;
    case Token::Type::String:
        value.type = ConfigValue::Type::String;
        value.stringData = token.stringData;
        break;
    case Token::Type::Symbol:
        if (token.symbol == '{')
        {
            value.type = ConfigValue::Type::Object;
            value.object = ParseObject(tokenizer);
        }
        else if (token.symbol == '[')
        {
            if (!ParseArray(tokenizer, value))
                return false;
        }
        else
        {
            LOG_ERROR("Unexpected symbol");
            return false;
        }
        break;
    default:
        LOG_ERROR("Unexpected token");
        return false;
    }

    return true;
}

bool Config::ParseArray(ConfigTokenizer& tokenizer, ConfigValue& value)
{
    ConfigArray array;
    int line, column;
    Token token;

    for (;;)
    {
        // step 1: read key name (or end of object)
        if (!tokenizer.GetToken(token, line, column)) // end of file
        {
            LOG_ERROR("End of file occured inside an array");
            return false;
        }
        if (token.type == Token::Type::Symbol && token.symbol == ']') // end of array
            break;

        // step 2: parse value
        ConfigValue val;
        if (!ParseValue(tokenizer, token, val))
            return false;

        // step 3: add the value to the array
        AddValue(array, val);
    }

    value.type = ConfigValue::Type::Array;
    value.array = array.mHead;
    return true;
}

ConfigObjectNodePtr Config::ParseObject(ConfigTokenizer& tokenizer)
{
    const char* key;
    ConfigObject object;
    int line, column;
    Token token;

    for (;;)
    {
        // step 1: read key name (or end of object)
        if (!tokenizer.GetToken(token, line, column)) // end of file
            return object.mHead;
        if (token.type == Token::Type::Symbol && token.symbol == '}') // end of object
            return object.mHead;

        if (token.type == Token::Type::Identifier) // key (node) name
            key = token.stringData;
        else
        {
            LOG_ERROR("Unexpected token at %i:%i.", line, column);
            return INVALID_NODE_PTR;
        }

        // step 2: read "=" sign
        if (!tokenizer.GetToken(token, line, column))
        {
            LOG_ERROR("Unexpected end of file");
            return INVALID_NODE_PTR;
        }
        if (token.type != Token::Type::Symbol && token.symbol != '=')
        {
            LOG_ERROR("Unexpected token at %i:%i. '=' expected", line, column);
            return INVALID_NODE_PTR;
        }

        // step 3: read value
        if (!tokenizer.GetToken(token, line, column))
        {
            LOG_ERROR("Unexpected end of file, value expected");
            return INVALID_NODE_PTR;
        }

        // step 4: parse value
        ConfigValue val;
        if (!ParseValue(tokenizer, token, val))
            return INVALID_NODE_PTR;

        // step 5: add value to the object
        AddValue(object, key, val);
    }
}

bool Config::ParseInPlace(char* string)
{
    ConfigTokenizer tokenizer(string);
    mRootNode = ParseObject(tokenizer);
    return mRootNode != INVALID_NODE_PTR;
}

void Config::Iterate(const ObjectIterator& callback, ConfigObjectNodePtr node) const
{
    if (node == INVALID_NODE_PTR)
        node = GetRootNode();
    else if (node >= mObjectNodes.size())
    {
        LOG_ERROR("Invalid 'node'");
        return;
    }

    while (node != INVALID_NODE_PTR)
    {
        const ConfigObjectNode& objectNode = mObjectNodes[node];
        if (!callback(objectNode.name, mValues[objectNode.valuePtr]))
            break;
        node = objectNode.next;
    }
}

void Config::IterateArray(const ArrayIterator& callback, ConfigArrayNodePtr node) const
{
    if (node >= mArrayNodes.size())
    {
        LOG_ERROR("Invalid 'node'");
        return;
    }

    int index = 0;
    while (node != INVALID_NODE_PTR)
    {
        const ConfigArrayNode& arrayNode = mArrayNodes[node];
        if (!callback(index++, mValues[arrayNode.valuePtr]))
            break;
        node = arrayNode.next;
    }
}

const char* INDENT = "  ";

void Config::ValueToString(std::stringstream& out, ConfigValuePtr valuePtr, int indent) const
{
    // TODO: this stringstream implementation is quite slow (it's slower than parsing),
    //       consider optimization

    const ConfigValue& value = mValues[valuePtr];

    int nextLevelIndent = indent >= 0 ? (indent + 1) : indent;
    const char* SPACING = (indent >= 0) ? " " : "";

    switch (value.type)
    {
    case ConfigValue::Type::Bool:
        out << SPACING << (value.boolData ? "true" : "false");
        break;
    case ConfigValue::Type::Int:
        out << SPACING << value.intData;
        break;
    case ConfigValue::Type::Float:
        out << SPACING << value.floatData;
        break;
    case ConfigValue::Type::String:
        out << SPACING << '"' << value.stringData << '"';
        break;
    case ConfigValue::Type::Object:
        if (indent >= 0)
            out << '\n';
        for (int i = 0; i < indent; ++i)
            out << INDENT;
        out << '{';
        if (indent >= 0)
            out << '\n';
        ObjectToString(out, value.object, nextLevelIndent);
        for (int i = 0; i < indent; ++i)
            out << INDENT;
        out << '}';
        break;
    case ConfigValue::Type::Array:
        {
            out << SPACING << '[';
            ConfigArrayNodePtr arrayNodePtr = value.array;
            while (arrayNodePtr != INVALID_NODE_PTR)
            {
                const ConfigArrayNode& arrayNode = mArrayNodes[arrayNodePtr];
                ValueToString(out, arrayNode.valuePtr, nextLevelIndent);
                arrayNodePtr = arrayNode.next;
                if (indent < 0 && arrayNodePtr != INVALID_NODE_PTR)
                    out << ' ';
            }
            out << SPACING << ']';
            break;
        }
    default:
        out << SPACING << "(Unknown)";
        break;
    }
}

void Config::ObjectToString(std::stringstream& out, ConfigObjectNodePtr objectPtr,
                            int indent) const
{
    while (objectPtr != INVALID_NODE_PTR)
    {
        for (int i = 0; i < indent; ++i)
            out << INDENT;

        const ConfigObjectNode& objectNode = mObjectNodes[objectPtr];
        out << objectNode.name;
        out << (indent >= 0 ? " =" : "=");
        ValueToString(out, objectNode.valuePtr, indent);
        objectPtr = objectNode.next;

        if (indent >= 0)
            out << '\n';
        else if (objectPtr != INVALID_NODE_PTR)
            out << ' ';
    }
}

String Config::ToString(bool format) const
{
    std::stringstream stringStream;
    ObjectToString(stringStream, GetRootNode(), format ? 0 : -1);

    // TODO ObjectToString should write to string
    return String(stringStream.str().c_str());
}


//
// ConfigGenericValue definitions
//

ConfigGenericValue::ConfigGenericValue(const Config* config)
    : ConfigValue(), mConfig(config)
{
    type = Type::Object;
    object = mConfig->GetRootNode();
}

ConfigGenericValue::ConfigGenericValue(const Config* config, const ConfigValue& val)
    : ConfigValue(val), mConfig(config)
{
}

bool ConfigGenericValue::HasMember(const char* key) const
{
    if (!IsObject())
        return false;

    bool found = false;
    auto callback = [&] (const char* iteratorKey, const ConfigValue&)
    {
        if (strcmp(iteratorKey, key) == 0)
        {
            found = true;
            return false;
        }
        return true;
    };

    mConfig->Iterate(callback, GetObj());
    return found;
}

ConfigGenericValue ConfigGenericValue::operator[](const char* key) const
{
    if (!IsObject())
        return ConfigGenericValue();

    ConfigValue valueFound;
    auto callback = [&] (const char* iteratorKey, const ConfigValue& value)
    {
        if (strcmp(iteratorKey, key) == 0)
        {
            valueFound = value;
            return false;
        }
        return true;
    };

    mConfig->Iterate(callback, GetObj());
    return ConfigGenericValue(mConfig, valueFound);
}

size_t ConfigGenericValue::GetSize() const
{
    if (!IsArray())
        return 0;

    size_t arraySize = 0;
    auto callback = [&] (int, const ConfigValue&)
    {
        arraySize++;
        return true;
    };

    mConfig->IterateArray(callback, GetArray());
    return arraySize;
}

ConfigGenericValue ConfigGenericValue::operator[](int index) const
{
    if (!IsArray())
        return ConfigGenericValue();

    ConfigValue valueFound;
    auto callback = [&] (int iteratorIndex, const ConfigValue& value)
    {
        if (iteratorIndex == index)
        {
            valueFound = value;
            return false;
        }
        return true;
    };

    mConfig->IterateArray(callback, GetArray());
    return ConfigGenericValue(mConfig, valueFound);
}

} // namespace Common
} // namespace NFE
