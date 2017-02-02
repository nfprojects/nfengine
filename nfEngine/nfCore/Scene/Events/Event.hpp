/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"


namespace NFE {
namespace Scene {

using EventTypeID = uint16;

/**
 * Base class for all event types.
 */
class CORE_API Event
{
public:
    virtual ~Event() { }

    // get event type ID
    EventTypeID GetThisID() const { return mTypeID; }

protected:
    Event(EventTypeID typeID)
        : mTypeID(typeID)
    { }

    // TODO this is temporary - it will be replaced with generic RTTI system
    static EventTypeID AllocateEventTypeID();

    const EventTypeID mTypeID;
};

} // namespace Scene
} // namespace NFE
