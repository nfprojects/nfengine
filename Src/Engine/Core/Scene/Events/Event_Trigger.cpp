/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "PCH.hpp"
#include "Event_Trigger.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Scene::Event_Trigger)
    NFE_CLASS_PARENT(NFE::Scene::Event)
NFE_END_DEFINE_CLASS()


namespace NFE {
namespace Scene {

Event_Trigger::Event_Trigger(Entity& sourceEntity, Entity& targetEntity, Type type)
    : mSourceEntity(sourceEntity)
    , mTargetEntity(targetEntity)
    , mType(type)
{ }


} // namespace Scene
} // namespace NFE
