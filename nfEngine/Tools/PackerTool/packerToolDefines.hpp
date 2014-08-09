#pragma once

#ifdef WIN32
#include <Windows.h>
#endif

//STL global definitions
#include <iostream>
#include <string>
#include <algorithm>

//Exception definition
#include "packerToolException.hpp"

#define SAFE_MEMFREE(x) if(x) { delete x; x = 0; }
#define SAFE_TABLEFREE(x) if(x) { delete[] x; x = 0; }
#define SAFE_FILEFREE(x) if(x) { fclose(x); x = 0; }

extern std::string s_configFilePath;

//A color in console takes eight different values: Red, Green, Blue and their
//combinations. Those colors are stored in a three bit value and mixed together.
//Unfortunately the order of the bytes is different under Linux and Windows:
//    * For Windows within these three bytes MSB is RED, LSB is BLUE
//    * For Linux and it's escape sequences MSB is BLUE, LSB is RED
//Instead of playing and rotating these three bytes a preprocessor will choose
//correct sequence inside this enum.
enum class ConsoleColor : unsigned short
{
    BLACK = 0,
#ifdef WIN32
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    YELLOW,
#else
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
#endif
    WHITE,
    INTENSE
};

typedef std::underlying_type<ConsoleColor>::type ConsoleColorType;

inline ConsoleColor operator|(ConsoleColor a, ConsoleColor b)
{
    return static_cast<ConsoleColor>(static_cast<ConsoleColorType>(a) | static_cast<ConsoleColorType>
                                     (b));
}
inline ConsoleColor operator&(ConsoleColor a, ConsoleColor b)
{
    return static_cast<ConsoleColor>(static_cast<ConsoleColorType>(a) & static_cast<ConsoleColorType>
                                     (b));
}

void PrintColored(std::string text, ConsoleColor foreground);
