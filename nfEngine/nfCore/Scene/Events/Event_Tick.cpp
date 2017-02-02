/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "PCH.hpp"
#include "Event_Tick.hpp"


namespace NFE {
namespace Scene {

Event_Tick::Event_Tick(float timeDelta, uint64 frameNumber)
    : Event(GetTypeID())
    , mTimeDelta(timeDelta)
    , mFrameNumber(frameNumber)
{
}

EventTypeID Event_Tick::GetTypeID()
{
    static EventTypeID id = AllocateEventTypeID();
    return id;
}

} // namespace Scene
} // namespace NFE
