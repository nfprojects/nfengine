/**
 * @file   Linux/Console.cpp
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Console-related function specific for Linux
 */

#include "stdafx.hpp"
#include "Console.hpp"

namespace {
const std::string coloringEnding = "\033[0m";
} // namespace

namespace NFE {
namespace Common {

// TODO this file was created as stub. Implementation was NOT tested.
// When nfCommon will be ported to linux make sure it works correctly.
void PrintColored(const std::string& text, const ConsoleColor& foreground)
{
    //In Linux - form an escape sequence and print text surrounded with it
    //Escape sequences in ANSI code are:
    //    30 - 37 <- regular, three LSB are RGB components
    //    90 - 97 <- intense, three LSB are RGB components
    unsigned int initialColor = 30;
    if ((foreground & ConsoleColor::Intense) == ConsoleColor::Intense)
        initialColor = 90;

    initialColor += static_cast<ConsoleColorType>(foreground & ConsoleColor::White);
    std::string escapeSequence = "\033[" + std::to_string(initialColor) + "m";
    std::cout << escapeSequence << text << coloringEnding;
}

} // namespace Common
} // namespace NFE
