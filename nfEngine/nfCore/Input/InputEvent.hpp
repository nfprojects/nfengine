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
struct EventData
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


    NFE_INLINE EventData()
        : type(Type::Unknown)
        , name(nullptr)
        , axisValue(0.0f)
    { }

    NFE_INLINE EventData(Type type, const char* name, float axisValue = 0.0f)
        : type(type)
        , name(name)
        , axisValue(axisValue)
    { }
};

} // namespace Input
} // namespace NFE