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

// TODO this file was created as stub. Implementation was NOT tested.
// When nfCommon will be ported to linux make sure it works correctly.
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
    printf("\033[%um", initialColor);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    printf("\033[0m");
}

} // namespace Common
} // namespace NFE
