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
    : name(nullptr)
    , next(nullptr)
{
}

ConfigParser::ConfigParser()
    : mRootNode(nullptr)
{
}

ConfigObjectNode* ConfigParser::AllocateObjectNode()
{
    std::unique_ptr<ConfigObjectNode> node(new ConfigObjectNode);
    ConfigObjectNode* nodePtr = node.get();
    mObjectNodes.emplace_back(std::move(node));
    return nodePtr;
}

ConfigArrayNode* ConfigParser::AllocateArrayNode()
{
    std::unique_ptr<ConfigArrayNode> node(new ConfigArrayNode);
    ConfigArrayNode* nodePtr = node.get();
    mArrayNodes.emplace_back(std::move(node));
    return nodePtr;
}

bool ConfigParser::Parse(const char* string)
{
    size_t strLength = strlen(string) + 1;
    mStringCopy.reset(new char[strLength]);
    memcpy(mStringCopy.get(), string, strLength);

    return ParseInPlace(mStringCopy.get());
}

bool ConfigParser::ParseValue(ConfigTokenizer& tokenizer, const Token& token, ConfigValue& value)
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
            value.type = ConfigValue::Type::Array;
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

bool ConfigParser::ParseArray(ConfigTokenizer& tokenizer, ConfigValue& value)
{
    ConfigArrayNode* prevNode = nullptr;
    value.array = nullptr;
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

        ConfigArrayNode* arrayNode = AllocateArrayNode();
        if (prevNode == nullptr)
            value.array = arrayNode;
        else
            prevNode->next = arrayNode;
        prevNode = arrayNode;

        if (!ParseValue(tokenizer, token, arrayNode->value))
            return false;
    }

    return true;
}

ConfigObjectNode* ConfigParser::ParseObject(ConfigTokenizer& tokenizer)
{
    ConfigObjectNode* retNode = nullptr;
    ConfigObjectNode* prevNode = nullptr;
    int line, column;
    Token token;

    for (;;)
    {
        // step 1: read key name (or end of object)
        if (!tokenizer.GetToken(token, line, column)) // end of file
            return retNode;
        if (token.type == Token::Type::Symbol && token.symbol == '}') // end of object
            return retNode;

        ConfigObjectNode* node = AllocateObjectNode();
        if (prevNode == nullptr) // build list of nodes
            retNode = node;
        else
            prevNode->next = node;
        prevNode = node;

        if (token.type == Token::Type::Identifier) // key (node) name
            node->name = token.stringData;
        else
        {
            LOG_ERROR("Unexpected token at %i:%i.", line, column);
            return nullptr;
        }

        // step 2: read "=" sign
        if (!tokenizer.GetToken(token, line, column))
        {
            LOG_ERROR("Unexpected end of file");
            return nullptr;
        }
        if (token.type != Token::Type::Symbol && token.symbol != '=')
        {
            LOG_ERROR("Unexpected token at %i:%i. '=' expected", line, column);
            return nullptr;
        }

        // step 3: read value
        if (!tokenizer.GetToken(token, line, column))
        {
            LOG_ERROR("Unexpected end of file, value expected");
            return nullptr;
        }
        if (!ParseValue(tokenizer, token, node->value))
            return nullptr;
    }
}

bool ConfigParser::ParseInPlace(char* string)
{
    ConfigTokenizer tokenizer(string);
    mRootNode = ParseObject(tokenizer);
    return mRootNode != nullptr;
}

void ConfigParser::Iterate(const InteratorCallback& callback, const ConfigObjectNode* node) const
{
    if (node == nullptr)
        node = GetRootNode();

    while (node != nullptr)
    {
        callback(node->name, node->value);
        node = node->next;
    }
}

ConfigObject* ConfigParser::CreateObject()
{
    // TODO
    return nullptr;
}

ConfigArray* ConfigParser::CreateArray(const ConfigValue* values, size_t numValues)
{
    (void)values;
    (void)numValues;
    // TODO
    return nullptr;
}

bool ConfigParser::AddToObject(ConfigObject* object, const char* keyName, const ConfigValue& value)
{
    (void)object;
    (void)keyName;
    (void)value;
    // TODO
    return true;
}

void ConfigParser::DebugPrintValue(const ConfigValue& value, int indent) const
{
    switch (value.type)
    {
    case ConfigValue::Type::Int:
        printf("(Int) %i\n", value.intData);
        break;
    case ConfigValue::Type::Bool:
        printf("(Bool) %s\n", value.boolData ? "true" : "false");
        break;
    case ConfigValue::Type::Float:
        printf("(Float) %f\n", value.floatData);
        break;
    case ConfigValue::Type::String:
        printf("(String) \"%s\"\n", value.stringData);
        break;
    case ConfigValue::Type::Object:
        printf("(Object)\n");
        DebugPrint(value.object, indent + 1);
        break;
    case ConfigValue::Type::Array:
        {
            printf("(Array)\n");
            ConfigArrayNode* arrayNode = value.array;
            while (arrayNode != nullptr)
            {
                for (int i = 0; i <= indent; ++i)
                    printf("    ");
                DebugPrintValue(arrayNode->value, indent + 1);
                arrayNode = arrayNode->next;
            }
            break;
        }
    default:
        printf("(Unknown)\n");
        break;
    }
}

void ConfigParser::DebugPrint(const ConfigObjectNode* node, int indent) const
{
    while (node != nullptr)
    {
        // print indent
        for (int i = 0; i < indent; ++i)
            printf("    ");

        printf("%s = ", node->name);
        DebugPrintValue(node->value, indent);
        node = node->next;
    }
}


} // namespace Common
} // namespace NFE
