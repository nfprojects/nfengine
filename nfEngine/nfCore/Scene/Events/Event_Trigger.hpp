/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "Event.hpp"


namespace NFE {
namespace Scene {

/**
 * Trigger enter/leave event.
 */
class CORE_API Event_Trigger : public Event
{
    NFE_DECLARE_POLYMORPHIC_CLASS(Event_Trigger)

public:
    enum class Type
    {
        Enter,  // two trigger components start interacting
        Leave,  // two trigger components stops interacting
    };

    explicit Event_Trigger(Entity& sourceEntity, Entity& targetEntity, Type type);

    /**
     * Get entity that possesses source trigger.
     */
    Entity& GetSourceEntity() const { return mSourceEntity; }

    /**
     * Get entity that possesses target trigger.
     */
    Entity& GetTargetEntity() const { return mTargetEntity; }

    /**
     * Get event type.
     */
    Type GetType() const { return mType; }

private:
    Entity& mSourceEntity;
    Entity& mTargetEntity;
    Type mType;
};


} // namespace Scene
} // namespace NFE
