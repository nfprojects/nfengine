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


class CORE_API IEntityController
{
    NFE_MAKE_NONCOPYABLE(IEntityController);

public:
    IEntityController();
    virtual ~IEntityController();

    void Attach(Entity* entity);
    void Detach();

    // get entity the controller is attached to
    NFE_INLINE Entity* GetEntity() const { return mEntity; }

    /**
     * Called by the engine when the entity receives an event.
     */
    virtual void OnEvent(const Event& event) = 0;

private:
    Entity* mEntity;
};

} // namespace Scene
} // namespace NFE
