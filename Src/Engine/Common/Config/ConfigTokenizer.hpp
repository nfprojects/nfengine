/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Config tokenizer declaration.
 */

#pragma once
#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

struct Token
{
    enum class Type
    {
        Integer,
        Float,
        Bool,
        Identifier,
        String,
        Symbol,
    };

    union
    {
        int integerData;
        float floatData;
        bool booleanData;
        const char* stringData;
        char symbol;
    };

    Type type;
};

class ConfigTokenizer
{
    enum class State
    {
        None, IdentOrNumber, String,
    };

    // string being tokenized
    char* mString;

    // string position tracking
    int mLine;
    int mColumn;

    // comments and whitespaces states
    bool ignoreWhitespace;
    bool commentStart;
    bool singleLineComment;
    bool multiLineComment;
    bool commentEnd;

    State state;
    const char* stringStart;
    char pendingSymbol;

public:
    ConfigTokenizer(char* string);
    bool GetToken(Token& token, int& line, int& column);
};



} // namespace Common
} // namespace NFE
