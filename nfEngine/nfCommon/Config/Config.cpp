/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser definition.
 */

#include "PCH.hpp"
#include "Config.hpp"
#include "ConfigTokenizer.hpp"
#include "Logger/Logger.hpp"
#include "Utils/StringUtils.hpp"


namespace NFE {
namespace Common {

Config::Config()
    : mRootNode(INVALID_NODE_PTR)
{
}

void Config::Clear()
{
    mRootNode = INVALID_NODE_PTR;
    mValues.Clear();
    mObjectNodes.Clear();
    mArrayNodes.Clear();
}

void Config::SetRoot(const ConfigObject& rootObject)
{
    mRootNode = rootObject.mHead;
}

ConfigValuePtr Config::AllocateValue()
{
    ConfigValuePtr ptr = static_cast<ConfigValuePtr>(mValues.Size());
    mValues.PushBack(ConfigValue());
    return ptr;
}

ConfigObjectNodePtr Config::AllocateObjectNode()
{
    ConfigObjectNodePtr nodePtr = static_cast<ConfigObjectNodePtr>(mObjectNodes.Size());
    mObjectNodes.PushBack(ConfigObjectNode());
    return nodePtr;
}

ConfigArrayNodePtr Config::AllocateArrayNode()
{
    ConfigArrayNodePtr nodePtr = static_cast<ConfigArrayNodePtr>(mArrayNodes.Size());
    mArrayNodes.PushBack(ConfigArrayNode());
    return nodePtr;
}

Config& Config::AddValue(ConfigObject& configObject, StringView key, const ConfigValue& val)
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
    mStringCopy.Reset(new char[strLength]);
    memcpy(mStringCopy.Get(), string, strLength);
    return ParseInPlace(mStringCopy.Get());
}

bool Config::ParseValue(ConfigTokenizer& tokenizer, const Token& token, ConfigValue& value)
{
    switch (token.type)
    {
    case Token::Type::Integer:
        value.type = ConfigValue::Type::Int32;
        value.intData32 = token.integerData;
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
            NFE_LOG_ERROR("Unexpected symbol");
            return false;
        }
        break;
    default:
        NFE_LOG_ERROR("Unexpected token");
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
            NFE_LOG_ERROR("End of file occured inside an array");
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
        {
            key = token.stringData;
        }
        else
        {
            NFE_LOG_ERROR("Unexpected token at %i:%i.", line, column);
            return INVALID_NODE_PTR;
        }

        // step 2: read "=" sign
        if (!tokenizer.GetToken(token, line, column))
        {
            NFE_LOG_ERROR("Unexpected end of file");
            return INVALID_NODE_PTR;
        }
        if (token.type != Token::Type::Symbol && token.symbol != '=')
        {
            NFE_LOG_ERROR("Unexpected token at %i:%i. '=' expected", line, column);
            return INVALID_NODE_PTR;
        }

        // step 3: read value
        if (!tokenizer.GetToken(token, line, column))
        {
            NFE_LOG_ERROR("Unexpected end of file, value expected");
            return INVALID_NODE_PTR;
        }

        // step 4: parse value
        ConfigValue val;
        if (!ParseValue(tokenizer, token, val))
            return INVALID_NODE_PTR;

        // step 5: add value to the object
        AddValue(object, StringView(key), val);
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
    else if (node >= mObjectNodes.Size())
    {
        NFE_LOG_ERROR("Invalid 'node'");
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
    if (node >= mArrayNodes.Size())
    {
        NFE_LOG_ERROR("Invalid 'node'");
        return;
    }

    uint32 index = 0;
    while (node != INVALID_NODE_PTR)
    {
        const ConfigArrayNode& arrayNode = mArrayNodes[node];
        if (!callback(index++, mValues[arrayNode.valuePtr]))
            break;
        node = arrayNode.next;
    }
}

const char* INDENT = "  ";

void Config::ValueToString(String& out, ConfigValuePtr valuePtr, int indent) const
{
    const ConfigValue& value = mValues[valuePtr];

    int nextLevelIndent = indent >= 0 ? (indent + 1) : indent;
    const char* SPACING = (indent >= 0) ? " " : "";

    switch (value.type)
    {
    case ConfigValue::Type::Bool:
        out += SPACING;
        out += (value.boolData ? "true" : "false");
        break;
    case ConfigValue::Type::Int8:
        out += SPACING;
        out += Common::ToString(static_cast<int32>(value.intData8));
        break;
    case ConfigValue::Type::Uint8:
        out += SPACING;
        out += Common::ToString(static_cast<uint32>(value.uintData8));
        break;
    case ConfigValue::Type::Int16:
        out += SPACING;
        out += Common::ToString(value.intData16);
        break;
    case ConfigValue::Type::Uint16:
        out += SPACING;
        out += Common::ToString(value.uintData16);
        break;
    case ConfigValue::Type::Int32:
        out += SPACING;
        out += Common::ToString(value.intData32);
        break;
    case ConfigValue::Type::Uint32:
        out += SPACING;
        out += Common::ToString(value.uintData32);
        break;
    case ConfigValue::Type::Int64:
        out += SPACING;
        out += Common::ToString(value.intData64);
        break;
    case ConfigValue::Type::Uint64:
        out += SPACING;
        out += Common::ToString(value.uintData64);
        break;
    case ConfigValue::Type::Float:
        out += SPACING;
        out += Common::ToString(value.floatData);
        break;
    case ConfigValue::Type::Double:
        out += SPACING;
        out += Common::ToString(value.doubleData);
        break;
    case ConfigValue::Type::String:
        out += SPACING;
        out += '\"';
        out += value.stringData;
        out += '\"';
        break;
    case ConfigValue::Type::Object:
        if (indent >= 0)
            out += '\n';
        for (int i = 0; i < indent; ++i)
            out += INDENT;
        out += '{';
        if (indent >= 0)
            out += '\n';
        ObjectToString(out, value.object, nextLevelIndent);
        for (int i = 0; i < indent; ++i)
            out += INDENT;
        out += '}';
        break;
    case ConfigValue::Type::Array:
    {
        out += SPACING;
        out += '[';
        ConfigArrayNodePtr arrayNodePtr = value.array;
        while (arrayNodePtr != INVALID_NODE_PTR)
        {
            const ConfigArrayNode& arrayNode = mArrayNodes[arrayNodePtr];
            ValueToString(out, arrayNode.valuePtr, nextLevelIndent);
            arrayNodePtr = arrayNode.next;
            if (indent < 0 && arrayNodePtr != INVALID_NODE_PTR)
                out += ' ';
        }
        out += SPACING;
        out += ']';
        break;
    }
    default:
        out += SPACING;
        out += "(Unknown)";
        break;
    }
}

void Config::ObjectToString(String& out, ConfigObjectNodePtr objectPtr, int indent) const
{
    while (objectPtr != INVALID_NODE_PTR)
    {
        for (int i = 0; i < indent; ++i)
            out += INDENT;

        const ConfigObjectNode& objectNode = mObjectNodes[objectPtr];
        out += objectNode.name;
        out += (indent >= 0 ? " =" : "=");
        ValueToString(out, objectNode.valuePtr, indent);
        objectPtr = objectNode.next;

        if (indent >= 0)
            out += '\n';
        else if (objectPtr != INVALID_NODE_PTR)
            out += ' ';
    }
}

String Config::ToString(bool format) const
{
    String str;
    ObjectToString(str, GetRootNode(), format ? 0 : -1);
    return str;
}


} // namespace Common
} // namespace NFE
