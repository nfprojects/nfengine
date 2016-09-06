/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Input events declarations.
 */

#pragma once

#include "../Core.hpp"

namespace NFE {
namespace Input {

struct Event
{
    enum class Type
    {
        KeyPress,
        KeyRelease,
        Axis
    };

    Type type;
    std::string name;
    float value;
};

} // namespace Input
} // namespace NFE