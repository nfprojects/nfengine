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
    NFE_DECLARE_POLYMORPHIC_CLASS(Event_Input);

public:
    Event_Input(const Input::EventData& data);
    const Input::EventData& GetData() const { return mData; }

private:
    Input::EventData mData;
};

} // namespace Scene
} // namespace NFE

NFE_DECLARE_TYPE(NFE::Scene::Event_Input);
