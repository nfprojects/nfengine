/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config parser definition.
 */

#include "PCH.hpp"
#include "Config.hpp"
#include "Logger.hpp"

namespace NFE {
namespace Common {

ConfigNode::ConfigNode()
    : mName(nullptr), stringData(nullptr), mType(Type::None)
{
}

ConfigParser::ConfigParser()
{
}

ConfigNode* ConfigParser::AllocateNode()
{
    std::unique_ptr<ConfigNode> node(new ConfigNode);
    ConfigNode* nodePtr = node.get();
    mNodes.emplace_back(std::move(node));
    return nodePtr;
}

bool ConfigParser::Parse(const char* string)
{
    size_t strLength = strlen(string) + 1;
    mStringCopy.reset(new char[strLength]);
    memcpy(mStringCopy.get(), string, strLength);

    return ParseInPlace(mStringCopy.get());
}

bool ConfigParser::ParseInPlace(char* string)
{
    // parser states
    enum class ValueType
    {
        None,
        String,
        Number  // float, integer or bool
    };

    enum class ParserStage
    {
        Node,
        EqualsSign,
        Value
    };

    // int depth = 0;
    int line = 0;
    int column = 0;
    char c;
    const char* identStart = nullptr;  // identificator start
    const char* valueStart = nullptr;

    bool ignoreWhitespace = true;
    bool commentStart = false;
    bool singleLineComment = false;
    bool multiLineComment = false;
    bool commentEnd = false;
    ParserStage stage = ParserStage::Node;
    ValueType valueType = ValueType::None;

    while ((c = *string) != 0)
    {
        if (singleLineComment) // we are inside single line comment
        {
            if (c == '\n' || c == '\r') // single line comment end
                singleLineComment = false;
        }
        else if (multiLineComment) // we are inside multiline comment
        {
            if (c == '*')
                commentEnd = true;
            else if (commentEnd && c == '/')
            {
                commentEnd = false;
                multiLineComment = false;
            }
            else
                commentEnd = false;
        }
        else if (commentStart) // start of a comment
        {
            if (c == '/')
                singleLineComment = true;
            else if (c == '*')
                multiLineComment = true;
            else
            {
                LOG_ERROR("Unexpected character at %i:%i.", line, column);
                return false;
            }
            commentStart = false;
        }
        else if (c == '/') // possible start of a new comment
            commentStart = true;
        else if (ignoreWhitespace && isspace(c))
        {
        }
        else if (stage == ParserStage::Node)
        {
            if (c == '}')
            {
                // TODO
            }
            else if (c == '}')
            {
                // TODO
            }
            else if (isalnum(c) || c == '_')
            {
                if (identStart == nullptr)
                    identStart = string;
                ignoreWhitespace = false;
            }
            else
            {
                if (c == '=')
                {
                    stage = ParserStage::Value;
                    valueType = ValueType::None;
                }
                else if (isspace(c))
                    stage = ParserStage::EqualsSign;
                else
                {
                    LOG_ERROR("Unexpected character at %i:%i. '=' sign was expected.",
                              line, column);
                    return false;
                }

                *string = 0;
                ignoreWhitespace = true;
            }
        }
        else if (stage == ParserStage::EqualsSign)
        {
            if (c == '=')
            {
                stage = ParserStage::Value;
                valueType = ValueType::None;
            }
            else
            {
                LOG_ERROR("Unexpected character at %i:%i. '=' sign was expected.", line, column);
                return false;
            }
        }
        else if (stage == ParserStage::Value)
        {
            ignoreWhitespace = false;
            if (valueType == ValueType::None) // try to determine value type by the first character
            {
                if (c == '{') // beginning of a object definition
                {
                    // TODO
                }
                else if (c == '[') // beginning of an array
                {
                    // TODO
                }
                else if (c == '"') // string will always start with quotation mark
                {
                    valueType = ValueType::String;
                    valueStart = string + 1;
                }
                else if (isalnum(c) || c == '-' || c == '+' || c == '.')
                {
                    valueType = ValueType::Number;
                    valueStart = string;
                }
                else
                {
                    LOG_ERROR("Unexpected character at %i:%i", line, column);
                    return false;
                }
            }
            else if ((valueType == ValueType::String && c == '"') ||
                     (valueType != ValueType::String && isspace(c)))
            {
                *string = 0;
                LOG_INFO("key='%s', value='%s'", identStart, valueStart);

                identStart = nullptr;
                valueStart = nullptr;
                stage = ParserStage::Node;
                ignoreWhitespace = true;
            }
        }

        string++;
        column++;
        if (c == '\n' || c == '\r')
        {
            line++;
            column = 0;
        }
    }

    if (stage == ParserStage::Value)
    {
        *string = 0;
        LOG_INFO("key='%s', value='%s'", identStart, valueStart);
    }

    return true;
}

} // namespace Common
} // namespace NFE
