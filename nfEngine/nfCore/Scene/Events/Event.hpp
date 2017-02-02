/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../../Core.hpp"

#include <functional>

namespace NFE {
namespace Scene {

using EventID = uint16;

/**
 * Base class for all event types.
 */
class Event
{
public:
    EventID eventID;
    const void* data;
};

struct EventData_BeginGame
{
    std::string gameName;
};

struct EventData_EndGame
{
    std::string gameName;
};

/**
 * Tick event - send once every frame.
 */
struct EventData_Tick
{
    float deltaTime;
    uint32 frameNumber;
};


} // namespace Scene
} // namespace NFE
