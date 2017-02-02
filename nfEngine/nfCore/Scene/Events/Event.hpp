/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"

#include <functional>


namespace NFE {
namespace Scene {

using EventTypeID = uint16;

/**
 * Base class for all event types.
 */
// TODO this is not a best solution, but it's easiest for now
class CORE_API Event
{
public:
    virtual ~Event() { }

    // get event type ID
    EventTypeID GetID() const { return mTypeID; }

protected:
    Event(EventTypeID typeID)
        : mTypeID(typeID)
    { }

    const EventTypeID mTypeID;
};

//////////////////////////////////////////////////////////////////////////

class CORE_API Event_BeginGame : public Event
{
public:
    Event_BeginGame(const std::string& gameName);
    const std::string& GetGameName() const { return mGameName; }
    static EventTypeID GetTypeID();

private:
    std::string mGameName;
};

//////////////////////////////////////////////////////////////////////////

class CORE_API Event_EndGame : public Event
{
public:
    Event_EndGame(const std::string& gameName);
    const std::string& GetGameName() const { return mGameName; }
    static EventTypeID GetTypeID();

private:
    std::string mGameName;
};

//////////////////////////////////////////////////////////////////////////

/**
 * Tick event - send once every frame.
 */
class CORE_API Event_Tick : public Event
{
public:
    Event_Tick(float timeDelta, uint64 frameNumber);
    float GetTimeDelta() const { return mTimeDelta; }
    uint64 GetFrameNumber() const { return mFrameNumber; }
    static EventTypeID GetTypeID();

private:
    float mTimeDelta;
    uint64 mFrameNumber;
};


} // namespace Scene
} // namespace NFE
