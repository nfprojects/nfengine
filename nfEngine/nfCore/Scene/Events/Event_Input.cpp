/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "PCH.hpp"
#include "Event_Input.hpp"


NFE_BEGIN_DEFINE_POLYMORPHIC_CLASS_IN_NAMESPACE2(NFE, Scene, Event_Input);
    NFE_CLASS_PARENT(NFE::Scene::Event);
NFE_END_DEFINE_CLASS();


namespace NFE {
namespace Scene {

Event_Input::Event_Input(const Input::EventData& data)
    : mData(data)
{
}

} // namespace Scene
} // namespace NFE
