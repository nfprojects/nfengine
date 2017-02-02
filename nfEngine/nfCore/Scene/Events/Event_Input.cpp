/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "PCH.hpp"
#include "Event_Input.hpp"


namespace NFE {
namespace Scene {

Event_Input::Event_Input(const Input::EventData& data)
    : Event(GetTypeID())
    , mData(data)
{
}

EventTypeID Event_Input::GetTypeID()
{
    static EventTypeID id = AllocateEventTypeID();
    return id;
}


} // namespace Scene
} // namespace NFE
