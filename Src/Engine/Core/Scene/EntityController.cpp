#include "PCH.hpp"
#include "EntityController.hpp"


namespace NFE {
namespace Scene {

IEntityController::IEntityController()
    : mEntity(nullptr)
{}

IEntityController::~IEntityController()
{
    NFE_ASSERT(!mEntity, "Controller is still attached to an entity");
}

void IEntityController::Attach(Entity* entity)
{
    NFE_ASSERT(!mEntity, "The controller is already attached to an entity");
    NFE_ASSERT(entity, "Invalid entity");
    mEntity = entity;
}

void IEntityController::Detach()
{
    NFE_ASSERT(mEntity, "The controller is not attached to an entity");
    mEntity = nullptr;
}

} // namespace Scene
} // namespace NFE
