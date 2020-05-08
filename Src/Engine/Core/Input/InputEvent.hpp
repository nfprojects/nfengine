/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Input events declarations.
 */

#pragma once

#include "../Core.hpp"
#include "../../Common/Containers/String.hpp"
#include "../../Common/Containers/StringView.hpp"


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
    Common::String name;

    // used only for Type::Axis events
    float axisValue;


    EventData()
        : type(Type::Unknown)
        , axisValue(0.0f)
    { }

    EventData(Type type, Common::StringView name, float axisValue = 0.0f)
        : type(type)
        , name(name)
        , axisValue(axisValue)
    { }
};

} // namespace Input
} // namespace NFE