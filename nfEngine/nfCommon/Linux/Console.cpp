/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Console-related function specific for Linux
 */

#include "../PCH.hpp"
#include "Console.hpp"
#include <stdarg.h>

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

    const int MSG_LEN = 1024;
    char stackBuffer[MSG_LEN];
    std::unique_ptr<char[]> heapBuffer;
    char* formatted;

    va_list args;
    va_list argsCopy;
    va_start(args, format);
    va_copy(argsCopy, args);

    int len = vsnprintf(stackBuffer, MSG_LEN, format, args);
    if (len < 0)
    {
        //va_end(argsCopy);
        va_end(args);
        return;
    }

    if (len >= MSG_LEN)
    {
        heapBuffer.reset(new (std::nothrow) char[len + 1]);
        if (heapBuffer)
        {
            formatted = heapBuffer.get();
            vsnprintf(formatted, len + 1, format, argsCopy);
        }
    }
    else if (len > 0)
        formatted = stackBuffer;

    va_end(args);

    printf("\033[%um%s\033[0m", initialColor, formatted);
}

} // namespace Common
} // namespace NFE
