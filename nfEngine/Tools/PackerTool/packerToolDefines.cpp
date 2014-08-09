#include "packerToolDefines.hpp"

std::string s_configFilePath = "config.cfg";

#ifndef WIN32
const std::string s_endColoring = "\033[0m";
#endif

void PrintColored(std::string text, ConsoleColor foreground)
{
#ifdef WIN32
    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    GetConsoleScreenBufferInfo(hstdout, &console_info);
    SetConsoleTextAttribute(hstdout, static_cast<ConsoleColorType>(foreground));
    std::cout << text;
    SetConsoleTextAttribute(hstdout, console_info.wAttributes);
#else
    //In Linux - form an escape sequence and print text surrounded with it
    //Escape sequences in ANSI code are:
    //    30 - 37 <- regular, three LSB are RGB components
    //    90 - 97 <- intense, three LSB are RGB components
    unsigned int initialColor = 30;
    if ((foreground & ConsoleColor::INTENSE) == ConsoleColor::INTENSE)
        initialColor = 90;

    initialColor += static_cast<ConsoleColorType>(foreground & ConsoleColor::WHITE);
    std::string escapeSequence = "\033[" + std::to_string(initialColor) + "m";
    std::cout << escapeSequence << text << s_endColoring;
#endif
}