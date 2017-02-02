/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "PCH.hpp"
#include "Event.hpp"


namespace NFE {
namespace Scene {

namespace {

EventTypeID AllocateEventTypeID()
{
    static EventTypeID id = 0;
    return id++;
}

} // namespace


//////////////////////////////////////////////////////////////////////////


Event_BeginGame::Event_BeginGame(const std::string& gameName)
    : Event(GetTypeID())
    , mGameName(gameName)
{
}

EventTypeID Event_BeginGame::GetTypeID()
{
    static EventTypeID id = AllocateEventTypeID();
    return id;
}


//////////////////////////////////////////////////////////////////////////


Event_EndGame::Event_EndGame(const std::string& gameName)
    : Event(GetTypeID())
    , mGameName(gameName)
{
}

EventTypeID Event_EndGame::GetTypeID()
{
    static EventTypeID id = AllocateEventTypeID();
    return id;
}


//////////////////////////////////////////////////////////////////////////


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
