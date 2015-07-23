/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Scene entity definition.
 */

#include "PCH.hpp"
#include "Entity.hpp"
#include "SceneManager.hpp"
#include "../nfCommon/Memory.hpp"
#include "../nfCommon/InputStream.hpp"
#include "../nfCommon/OutputStream.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Scene {

using namespace Math;

Entity::Entity()
{
    mFlags = 0;
    mScene = nullptr;
    mSegment = nullptr;
    mUserPointer = nullptr;
}

Entity::~Entity()
{
    RemoveAllComponents();
}

Result Entity::SetName(const char* pNewName)
{
    // TODO: validate input, check if a name is not used
    mName = pNewName;
    return Result::OK;
}

const char* Entity::GetName() const
{
    return mName.c_str();
}

SceneManager* Entity::GetScene() const
{
    return mScene;
}

Segment* Entity::GetSceneSegment() const
{
    return mSegment;
}

void Entity::SetUserPointer(void* ptr)
{
    mUserPointer = ptr;
}

void* Entity::GetUserPointer() const
{
    return mUserPointer;
}

Result Entity::AddComponent(Component* pComponent)
{
    mComponents.insert(pComponent);
    return Result::OK;
}

Result Entity::RemoveComponent(Component* pComponent)
{
    if (mComponents.count(pComponent) == 0)
    {
        LOG_ERROR("Component is not attached to the entity!");
        return Result::Error;
    }

    //pComponent->mOwner = nullptrptr;
    mComponents.erase(pComponent);
    delete pComponent;
    return Result::OK;
}

void Entity::RemoveAllComponents()
{
    for (auto pComp : mComponents)
        delete pComp;

    mComponents.clear();
}

} // namespace Scene
} // namespace NFE
