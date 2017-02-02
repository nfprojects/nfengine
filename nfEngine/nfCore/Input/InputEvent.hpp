/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Input events declarations.
 */

#pragma once

#include "../Core.hpp"

namespace NFE {
namespace Input {

/**
 * Structure representing a mapped input event.
 */
struct Event
{
    enum class Type
    {
        Unknown,
        KeyPress,
        KeyRelease,
        Axis
    };

    // event type
    Type type;

    // event name (mapping name)
    const char* name;

    // used only for Type::Axis events
    float axisValue;


    NFE_INLINE Event()
        : type(Type::Unknown)
        , name(nullptr)
        , axisValue(0.0f)
    { }
};

} // namespace Input
} // namespace NFE