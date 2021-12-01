/**
 * @file
 * @author Lookey (costyrra.xl@gmail.com)
 * @brief  Implementation of Console-related function specific for Linux
 */

#pragma once

#include "nfCommon.hpp"

#include <type_traits>


namespace NFE {
namespace Common {

// A color in console takes eight different values: Red, Green, Blue and their
// combinations. Those colors are stored in a three bit value and mixed together.
// For Linux and it's escape sequences MSB is BLUE, LSB is RED.
enum class ConsoleColor : unsigned short
{
    Black = 0,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    Intense
};

typedef std::underlying_type<ConsoleColor>::type ConsoleColorType;

inline ConsoleColor operator|(const ConsoleColor& a, const ConsoleColor& b)
{
    return static_cast<ConsoleColor>(static_cast<ConsoleColorType>(a) | static_cast<ConsoleColorType>
                                     (b));
}
inline ConsoleColor operator&(const ConsoleColor& a, const ConsoleColor& b)
{
    return static_cast<ConsoleColor>(static_cast<ConsoleColorType>(a) & static_cast<ConsoleColorType>
                                     (b));
}

void PrintColored(const ConsoleColor& foreground, const char* format, ...);

} // namespace Common
} // namespace NFE
