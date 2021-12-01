/**
 * @file
 * @author Lookey (costyrra.xl@gmail.com)
 * @brief  Implementation of Console-related function specific for Linux
 */

#include "PCH.hpp"
#include "Console.hpp"

#include <cstdarg>
#include <cstdio>

#include "Containers/DynArray.hpp"


namespace NFE {
namespace Common {

void PrintColored(const ConsoleColor& foreground, const char* format, ...)
{
    // In Linux - form an escape sequence and print text surrounded with it
    // Escape sequences in ANSI code are:
    //    30 - 37 <- regular, three LSB are RGB components
    //    40 - 47 <- intense, three LSB are RGB components
    unsigned int initialColor = 30;
    if ((foreground & ConsoleColor::Intense) == ConsoleColor::Intense)
        initialColor = 40;

    initialColor += static_cast<ConsoleColorType>(foreground & ConsoleColor::White);

    const char* newFormat = "\033[%um%s\033[0m";
    const uint32_t formatLen = 1024;
    char formatBuffer[formatLen];
    Common::DynArray<char> heapFormatBuffer;
    char* formatted = nullptr;

    int actualStrLen = snprintf(formatBuffer, formatLen, newFormat, initialColor, format);
    if (actualStrLen < 0)
    {
        snprintf(formatBuffer, formatLen, "!!! Encoding error !!!");
    }
    else if (actualStrLen >= formatLen)
    {
        if (!heapFormatBuffer.Resize(actualStrLen + 1))
        {
            snprintf(formatBuffer, formatLen, "!!! Cannot allocate memory for provided format !!!");
        }

        formatted = heapFormatBuffer.Data();
        snprintf(formatted, actualStrLen, newFormat, initialColor, format);
        formatted[actualStrLen] = 0;
    }
    else
    {
        formatted = formatBuffer;
        formatted[actualStrLen] = 0;
    }

    va_list args;

    va_start(args, format);
    vfprintf(stdout, formatted, args);
    va_end(args);
}

} // namespace Common
} // namespace NFE
