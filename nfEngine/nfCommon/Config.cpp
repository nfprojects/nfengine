/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser definition.
 */

#include "PCH.hpp"
#include "Config.hpp"
#include "ConfigTokenizer.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Common {

ConfigObjectNode::ConfigObjectNode()
    : name(nullptr), next(INVALID_NODE_PTR)
{}

Config::Config()
    : mRootNode(INVALID_NODE_PTR)
{}

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

bool Config::Parse(const char* string)
{
    size_t strLength = strlen(string) + 1;
    mStringCopy.reset(new char[strLength]);
    memcpy(mStringCopy.get(), string, strLength);
    return ParseInPlace(mStringCopy.get());
}

bool Config::ParseValue(ConfigTokenizer& tokenizer, const Token& token,
                              ConfigValuePtr valuePtr)
{
    switch (token.type)
    {
    case Token::Type::Integer:
        mValues[valuePtr].type = ConfigValue::Type::Int;
        mValues[valuePtr].intData = token.integerData;
        break;
    case Token::Type::Float:
        mValues[valuePtr].type = ConfigValue::Type::Float;
        mValues[valuePtr].floatData = token.floatData;
        break;
    case Token::Type::Bool:
        mValues[valuePtr].type = ConfigValue::Type::Bool;
        mValues[valuePtr].boolData = token.booleanData;
        break;
    case Token::Type::String:
        mValues[valuePtr].type = ConfigValue::Type::String;
        mValues[valuePtr].stringData = token.stringData;
        break;
    case Token::Type::Symbol:
        if (token.symbol == '{')
        {
            mValues[valuePtr].type = ConfigValue::Type::Object;
            mValues[valuePtr].object = ParseObject(tokenizer);
        }
        else if (token.symbol == '[')
        {
            mValues[valuePtr].type = ConfigValue::Type::Array;
            if (!ParseArray(tokenizer, valuePtr))
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

bool Config::ParseArray(ConfigTokenizer& tokenizer, ConfigValuePtr valuePtr)
{
    ConfigArrayNodePtr prevNode = INVALID_NODE_PTR;
    mValues[valuePtr].array = INVALID_NODE_PTR;
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

        // step 2: update array linked list
        ConfigArrayNodePtr arrayNode = AllocateArrayNode();
        if (prevNode == INVALID_NODE_PTR)
            mValues[valuePtr].array = arrayNode;
        else
            mArrayNodes[prevNode].next = arrayNode;
        prevNode = arrayNode;

        // step 3: allocate value for array and parse it
        ConfigValuePtr newValuePtr = AllocateValue();
        mArrayNodes[prevNode].valuePtr = newValuePtr;
        if (!ParseValue(tokenizer, token, newValuePtr))
            return false;
    }

    return true;
}

ConfigObjectNodePtr Config::ParseObject(ConfigTokenizer& tokenizer)
{
    ConfigObjectNodePtr retNode = INVALID_NODE_PTR;
    ConfigObjectNodePtr prevNode = INVALID_NODE_PTR;
    int line, column;
    Token token;

    for (;;)
    {
        // step 1: read key name (or end of object)
        if (!tokenizer.GetToken(token, line, column)) // end of file
            return retNode;
        if (token.type == Token::Type::Symbol && token.symbol == '}') // end of object
            return retNode;

        ConfigObjectNodePtr node = AllocateObjectNode();
        if (prevNode == INVALID_NODE_PTR) // build list of nodes
            retNode = node;
        else
            mObjectNodes[prevNode].next = node;
        prevNode = node;

        if (token.type == Token::Type::Identifier) // key (node) name
            mObjectNodes[node].name = token.stringData;
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

        // step 4: allocate value for node and parse it
        ConfigValuePtr valuePtr = AllocateValue();
        mObjectNodes[node].valuePtr = valuePtr;
        if (!ParseValue(tokenizer, token, valuePtr))
            return INVALID_NODE_PTR;
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

    while (node != INVALID_NODE_PTR)
    {
        const ConfigObjectNode& objectNode = mObjectNodes[node];
        callback(objectNode.name, mValues[objectNode.valuePtr]);
        node = objectNode.next;
    }
}

ConfigObject* Config::CreateObject()
{
    // TODO
    return nullptr;
}

ConfigArray* Config::CreateArray(const ConfigValue* values, size_t numValues)
{
    (void)values;
    (void)numValues;
    // TODO
    return nullptr;
}

bool Config::AddValue(ConfigObject* object, const char* keyName, const ConfigValue& value)
{
    (void)object;
    (void)keyName;
    (void)value;
    // TODO
    return true;
}

void Config::ValueToString(std::string& outputStr, ConfigValuePtr valuePtr) const
{
    // TODO: formatting

    const ConfigValue& value = mValues[valuePtr];

    switch (value.type)
    {
    case ConfigValue::Type::Int:
        outputStr += std::to_string(value.intData);
        break;
    case ConfigValue::Type::Bool:
        outputStr += value.boolData ? "true" : "false";
        break;
    case ConfigValue::Type::Float:
        outputStr += std::to_string(value.floatData);
        break;
    case ConfigValue::Type::String:
        outputStr += '"';
        outputStr += value.stringData;
        outputStr += '"';
        break;
    case ConfigValue::Type::Object:
        outputStr += "{ ";
        ObjectToString(outputStr, value.object);
        outputStr += "}";
        break;
    case ConfigValue::Type::Array:
        {
            outputStr += "[ ";
            ConfigArrayNodePtr arrayNode = value.array;
            while (arrayNode != INVALID_NODE_PTR)
            {
                ValueToString(outputStr, mArrayNodes[value.array].valuePtr);
                arrayNode = mArrayNodes[arrayNode].next;
                outputStr += ' ';
            }
            outputStr += ']';
            break;
        }
    default:
        printf("(Unknown)\n");
        break;
    }
}

void Config::ObjectToString(std::string& outputStr, ConfigObjectNodePtr objectPtr) const
{
    // TODO: formatting

    while (objectPtr != INVALID_NODE_PTR)
    {
        const ConfigObjectNode& objectNode = mObjectNodes[objectPtr];
        outputStr += objectNode.name;
        outputStr += " = ";
        ValueToString(outputStr, objectNode.valuePtr);
        outputStr += ' ';
        objectPtr = objectNode.next;
    }
}

std::string Config::ToString() const
{
    std::string str;
    ObjectToString(str, GetRootNode());
    return str;
}

} // namespace Common
} // namespace NFE
