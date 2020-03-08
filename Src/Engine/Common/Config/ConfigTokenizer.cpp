/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config tokenizer definition.
 */

#include "PCH.hpp"
#include "ConfigTokenizer.hpp"
#include "Logger/Logger.hpp"

namespace NFE {
namespace Common {

namespace {

bool IsSymbol(char c)
{
    return c == '=' || c == '{' || c == '}' || c == '[' || c == ']' || c == ',';
}

// check if a character may be part of an identifier or a number
bool IsIdentOrNumber(char c)
{
    return isalnum(c) || c == '_' || c == '-' || c == '+' || c == '.';
}

/**
 * Detect token type based on it's content.
 * Returns "false" on parsing failure.
 */
bool ParseToken(const char* str, Token& token)
{
    char c;

    if (strcmp(str, "true") == 0)
    {
        token.type = Token::Type::Bool;
        token.booleanData = true;
    }
    else if (strcmp(str, "false") == 0)
    {
        token.type = Token::Type::Bool;
        token.booleanData = false;
    }
    else if (isalpha(str[0]) || str[0] == '_') // identifiers begin with letter or underscore
    {
        token.type = Token::Type::Identifier;
        token.stringData = str;
        while ((c = *(++str)) != '\0')
            if (!isalnum(*str) && *str != '_')
                return false;
    }
    else // we have a number
    {
        const char* originalStr = str;
        bool comma = false;
        bool hexPrefix = (str[0] == '0') && (str[1] == 'x');
        while ((c = *(str++)) != '\0')
        {
            if (c == '.')
            {
                if (comma)
                    return false;
                comma = true;
            }
        }

        if (comma && hexPrefix) // invalid
            return false;
        else if (hexPrefix) // hexadecimal integer
        {
            token.type = Token::Type::Integer;
            token.integerData = static_cast<int>(strtoul(originalStr + 2, nullptr, 16));
        }
        else if (comma) // float
        {
            token.type = Token::Type::Float;
            token.floatData = strtof(originalStr, nullptr);
        }
        else // decimal integer
        {
            token.type = Token::Type::Integer;
            token.integerData = atoi(originalStr);
        }
    }

    return true;
}

} // namespace


ConfigTokenizer::ConfigTokenizer(char* string)
{
    mString = string;
    mLine = 0;
    mColumn = 0;

    ignoreWhitespace = true;
    commentStart = false;
    singleLineComment = false;
    multiLineComment = false;
    commentEnd = false;

    state = State::None;
    stringStart = nullptr;
    pendingSymbol = 0;
}

bool ConfigTokenizer::GetToken(Token& token, int& line, int& column)
{
    line = mLine;
    column = mColumn;

    char c;

    if (pendingSymbol != 0)
    {
        token.type = Token::Type::Symbol;
        token.symbol = pendingSymbol;
        pendingSymbol = 0;
        return true;
    }

    while ((c = *mString) != '\0')
    {
        // if set to true, a parsed token will be returned at the end of the loop
        bool returnToken = false;

        // strings have the highest priority (they can contain comment symbols)
        if (state == State::String)
        {
            if (c == '"') // TODO: escaping
            {
                *mString = 0;
                state = State::None;
                ignoreWhitespace = true;

                token.type = Token::Type::String;
                token.stringData = stringStart;
                returnToken = true;
            }
        }
        else if (singleLineComment) // we are inside single line comment
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
                NFE_LOG_ERROR("Unexpected character at %i:%i.", line, column);
                return false;
            }
            commentStart = false;
        }
        else if (c == '/') // possible start of a new comment
            commentStart = true;
        else if (ignoreWhitespace && isspace(c)) // ignore whitespace if required
            ;
        else if (state == State::None) // start parsing a new token
        {
            if (c == '"')
            {
                stringStart = mString + 1;
                state = State::String;
                ignoreWhitespace = false;
            }
            else if (IsIdentOrNumber(c))
            {
                stringStart = mString;
                state = State::IdentOrNumber;
                ignoreWhitespace = false;
            }
            else if (IsSymbol(c))
            {
                token.type = Token::Type::Symbol;
                token.symbol = c;
                returnToken = true;
            }
            else
            {
                NFE_LOG_ERROR("Unexpected character at %i:%i.", line, column);
                return false;
            }
        }
        else if (state == State::IdentOrNumber)
        {
            bool symbol = IsSymbol(c);
            if (isspace(c) || symbol) // end of identifier or number
            {
                *mString = 0;
                state = State::None;
                ignoreWhitespace = true;

                ParseToken(stringStart, token);
                returnToken = true;

                if (symbol) // we can't return two tokens so it's deferred
                    pendingSymbol = c;
            }
            else if (IsIdentOrNumber(c)) // continuation of identifier or number
                ;
            else
            {
                NFE_LOG_ERROR("Unexpected character at %i:%i.", line, column);
                return false;
            }
        }
        else
        {
            NFE_LOG_ERROR("Parse error at %i:%i.", line, column);
            return false;
        }

        mString++;
        mColumn++;
        if (c == '\n' || c == '\r')
        {
            mLine++;
            mColumn = 0;
        }

        if (returnToken)
            return true;
    }

    if (state == State::IdentOrNumber)
    {
        ParseToken(stringStart, token);
        state = State::None;
        return true;
    }
    else if (state == State::String)
    {
        token.type = Token::Type::String;
        token.stringData = stringStart;
        state = State::None;
        NFE_LOG_WARNING("String reached end of file without closing quotation mark");
        return true;
    }

    if (multiLineComment)
    {
        NFE_LOG_WARNING("Multiline comment reached end of file");
        multiLineComment = false;
    }

    return false; // end of file
}

} // namespace Common
} // namespace NFE
