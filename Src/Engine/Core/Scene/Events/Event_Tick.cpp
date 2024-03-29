/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "Event_Tick.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::Event_Tick)
    NFE_CLASS_PARENT(NFE::Scene::Event)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

Event_Tick::Event_Tick(float timeDelta, uint64 frameNumber)
    : mTimeDelta(timeDelta)
    , mFrameNumber(frameNumber)
{
}

} // namespace Scene
} // namespace NFE
