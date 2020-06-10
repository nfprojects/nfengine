/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of IEntityController.
 */

#pragma once

#include "../Core.hpp"
#include "Events/Event.hpp"


namespace NFE {
namespace Scene {


/**
 * Entity controller.
 *
 * The only class allowed to modify entity.
 */
class CORE_API IEntityController
{
    NFE_MAKE_NONCOPYABLE(IEntityController)

public:
    IEntityController();
    virtual ~IEntityController();

    void Attach(Entity* entity);
    void Detach();

    // get entity the controller is attached to
    Entity* GetEntity() const { return mEntity; }

    /**
     * Called by the engine when the entity receives an event.
     * @note This is the only allowed place, when entity modification is performed.
     */
    virtual void OnEvent(const Event& event) = 0;

private:
    Entity* mEntity;
};


} // namespace Scene
} // namespace NFE
