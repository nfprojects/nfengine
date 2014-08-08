/**
    NFEngine project

    \file   Component.cpp
    \brief  Component's base definition.
*/

#include "stdafx.h"
#include "Component.h"
#include "Entity.h"

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
