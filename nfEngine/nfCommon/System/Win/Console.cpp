/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Console-related function specific for Windows
 */

#include "PCH.hpp"
#include "Console.hpp"


namespace NFE {
namespace Common {

void PrintColored(const ConsoleColor& foreground, const char* format, ...)
{
    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    GetConsoleScreenBufferInfo(hstdout, &console_info);
    SetConsoleTextAttribute(hstdout, static_cast<ConsoleColorType>(foreground));

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    SetConsoleTextAttribute(hstdout, console_info.wAttributes);
}

} // namespace Common
} // namespace NFE
