/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "Event.hpp"


namespace NFE {
namespace Scene {

/**
 * Tick event - send once every frame.
 */
class CORE_API Event_Tick : public Event
{
    NFE_DECLARE_POLYMORPHIC_CLASS(Event_Tick)

public:
    explicit Event_Tick(float timeDelta, uint64 frameNumber);
    float GetTimeDelta() const { return mTimeDelta; }
    uint64 GetFrameNumber() const { return mFrameNumber; }

private:
    float mTimeDelta;
    uint64 mFrameNumber;
};


} // namespace Scene
} // namespace NFE
