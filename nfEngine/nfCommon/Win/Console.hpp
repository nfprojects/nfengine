/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Implementation of Console-related function specific for Windows
 */

#pragma once

#include "../nfCommon.hpp"

#include <type_traits>


namespace NFE {
namespace Common {

// A color in console takes eight different values: Red, Green, Blue and their
// combinations. Those colors are stored in a three bit value and mixed together.
// For Windows within these three bytes MSB is RED, LSB is BLUE
enum class ConsoleColor : unsigned short
{
    Black = 0,
    Blue,
    Green,
    Cyan,
    Red,
    Magenta,
    Yellow,
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

NFCOMMON_API void PrintColored(const ConsoleColor& foreground, const char* format, ...);

} // namespace Common
} // namespace NFE
