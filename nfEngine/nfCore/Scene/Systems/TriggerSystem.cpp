/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of trigger system.
 */

#include "PCH.hpp"
#include "TriggerSystem.hpp"
#include "Engine.hpp"


namespace NFE {
namespace Scene {

using namespace Math;


TriggerSystem::~TriggerSystem()
{
    NFE_ASSERT(mTriggers.Size() == 0,
               "Destroying trigger system while there are some registered triggers left. This may indicate a memory leak");
}

TriggerID TriggerSystem::CreateTrigger(const TriggerDesc& desc)
{
    TriggerObject object;
    object.box = desc.box;

    TriggerID id = mTriggers.Add(object);

    void* userData = reinterpret_cast<void*>(static_cast<size_t>(id));
    const uint32 treeNode = mTriggersTree.Insert(desc.box, userData);

    mTriggers[id].treeNodeId = treeNode;

    // we must find overlapping pairs in this volume
    InvalidateArea(desc.box);

    return id;
}

void TriggerSystem::UpdateTrigger(const TriggerID id, const TriggerDesc& newDesc)
{
    NFE_ASSERT(mTriggers.Has(id), "Invalid trigger ID");

    TriggerObject& desc = mTriggers[id];
    if (desc.box != newDesc.box)
    {
        InvalidateArea(desc.box);
        InvalidateArea(newDesc.box);
    }
}

void TriggerSystem::DestroyTrigger(const TriggerID id)
{
    NFE_ASSERT(mTriggers.Has(id), "Invalid trigger ID");

    const TriggerObject& desc = mTriggers[id];
    InvalidateArea(desc.box);

    mTriggersTree.Remove(desc.treeNodeId);
    mTriggers.Remove(id);
}

void TriggerSystem::InvalidateArea(const Math::Box& box)
{
    mTriggersTree.Query(box, [this](void* userData)
    {
        TriggerID triggerID = static_cast<TriggerID>(reinterpret_cast<size_t>(userData));
        mInvalidatedTriggers.insert(triggerID);
    });
}

void TriggerSystem::Update(float timeDelta)
{
    UNUSED(timeDelta);

    mInvalidatedTriggers.clear();
}

} // namespace Scene
} // namespace NFE
