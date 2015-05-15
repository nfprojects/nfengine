/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base definition.
 */

#include "PCH.hpp"
#include "Component.hpp"
#include "Entity.hpp"

namespace NFE {
namespace Scene {

Component::Component(Entity* pParent)
{
    mType = ComponentType::Unknown;
    mOwner = nullptr;
    mVirtual = false;

    pParent->AddComponent(this);
}

Component::~Component()
{
    /*
    if (mOwner)
    {
        mOwner->RemoveComponent(this);
        mOwner = nullptr;
    }*/
}

void Component::ReceiveMessage(ComponentMsg type, void* pData)
{
    // nothing to do by default
}

} // namespace Scene
} // namespace NFE
