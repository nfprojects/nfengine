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
struct Event
{
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

using EventCallback = std::function<void(const Event&)>;


} // namespace Scene
} // namespace NFE
