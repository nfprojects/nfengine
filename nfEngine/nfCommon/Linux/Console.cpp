/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Console-related function specific for Linux
 */

#include "PCH.hpp"
#include "Console.hpp"

namespace NFE {
namespace Common {

// TODO this file was created as stub. Implementation was NOT tested.
// When nfCommon will be ported to linux make sure it works correctly.
void PrintColored(const char* text, const ConsoleColor& foreground)
{
    // In Linux - form an escape sequence and print text surrounded with it
    // Escape sequences in ANSI code are:
    //    30 - 37 <- regular, three LSB are RGB components
    //    90 - 97 <- intense, three LSB are RGB components
    unsigned int initialColor = 30;
    if ((foreground & ConsoleColor::Intense) == ConsoleColor::Intense)
        initialColor = 90;

    initialColor += static_cast<ConsoleColorType>(foreground & ConsoleColor::White);
    printf("\\033[%um%s\\033[0m", initialColor, text);
}

} // namespace Common
} // namespace NFE
