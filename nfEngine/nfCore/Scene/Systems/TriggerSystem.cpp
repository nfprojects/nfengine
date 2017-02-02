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


TriggerID TriggerSystem::CreateTrigger(const TriggerDesc& desc)
{
    return mTriggers.Add(desc);
}

bool TriggerSystem::UpdateTrigger(const TriggerID id, const TriggerDesc& newDesc)
{
    NFE_ASSERT(mTriggers.Has(id), "Invalid trigger ID");

    TriggerDesc& desc = mTriggers[id];
    desc = newDesc;
    return true;
}

void TriggerSystem::DestroyTrigger(const TriggerID id)
{
    mTriggers.Remove(id);
}

void TriggerSystem::Update(float timeDelta)
{
    // mTriggers
}

} // namespace Scene
} // namespace NFE
