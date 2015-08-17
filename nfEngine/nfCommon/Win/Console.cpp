/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Console-related function specific for Windows
 */

#include "../PCH.hpp"
#include "Console.hpp"
#include <iostream>

namespace NFE {
namespace Common {

void PrintColored(const char* text, const ConsoleColor& foreground)
{
    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    GetConsoleScreenBufferInfo(hstdout, &console_info);
    SetConsoleTextAttribute(hstdout, static_cast<ConsoleColorType>(foreground));
    printf(text);
    SetConsoleTextAttribute(hstdout, console_info.wAttributes);
}

} // namespace Common
} // namespace NFE
