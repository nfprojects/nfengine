/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "PCH.hpp"
#include "Event.hpp"


namespace NFE {
namespace Scene {


EventTypeID Event::AllocateEventTypeID()
{
    static EventTypeID id = 0;
    return id++;
}


} // namespace Scene
} // namespace NFE
