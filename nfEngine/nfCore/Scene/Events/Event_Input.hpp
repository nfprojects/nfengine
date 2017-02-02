/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "Event.hpp"
#include "../../Input/InputEvent.hpp"

namespace NFE {
namespace Scene {

/**
 * Input event - triggered on an input action (key press, mouse move, etc.)
 */
class CORE_API Event_Input : public Event
{
public:
    Event_Input(const Input::EventData& data);
    const Input::EventData& GetData() const { return mData; }

    // TODO remove
    static EventTypeID GetTypeID();

private:
    Input::EventData mData;
};

} // namespace Scene
} // namespace NFE
