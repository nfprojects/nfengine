/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "Event_Input.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::Event_Input)
    NFE_CLASS_PARENT(NFE::Scene::Event)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

Event_Input::Event_Input(const Input::EventData& data)
    : mData(data)
{
}

} // namespace Scene
} // namespace NFE
