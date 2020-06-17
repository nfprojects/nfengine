/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of trigger system.
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"

#include "../../../Common/Containers/Map.hpp"
#include "../../../Common/Containers/HashSet.hpp"
#include "../../../Common/Memory/Aligned.hpp"
#include "../../../Common/Math/Vec4f.hpp"
#include "../../../Common/Math/Box.hpp"
#include "../../../Common/Utils/BVH.hpp"


namespace NFE {
namespace Scene {

// TODO
// everything here is a proof-of-concept

using TriggerID = uint32;

struct NFE_ALIGN(16) TriggerDesc
{
    Math::Box box;
    Entity* entity;
    bool isTarget;
};

struct TriggerInteraction
{
    TriggerID triggerA;
    TriggerID triggerB;

    bool operator == (const TriggerInteraction& rhs) const
    {
        return (triggerA == rhs.triggerA) && (triggerB == rhs.triggerB);
    }

    bool operator < (const TriggerInteraction& rhs) const
    {
        if (triggerA == rhs.triggerA)
            return triggerB < rhs.triggerB;

        return triggerA < rhs.triggerA;
    }
};

struct NFE_ALIGN(16) TriggerObject
{
    Math::Box box;
    Entity* entity;
    uint32 treeNodeId;
    TriggerID id;
    bool isTarget;
};

/**
 * Trigger system.
 */
class CORE_API NFE_ALIGN(16) TriggerSystem final
    : public ISystem
{
public:
    NFE_ALIGNED_CLASS(16)

    static const int ID = 5;

    explicit TriggerSystem(Scene& scene);
    ~TriggerSystem();

    /**
     * Update the trigger system.
     * @note    This will generate trigger events.
     */
    void Update(const SystemUpdateContext& context) override;

    /**
     * Create a new trigger object.
     * @notes   When trigger is created in overlapping state, it will generate "enter" event.
     */
    TriggerID CreateTrigger(const TriggerDesc& desc);

    /**
     * Update an existing trigger object.
     */
    void UpdateTrigger(const TriggerID id, const Math::Box& newBox);

    /**
     * Destroy an existing trigger object.
     */
    void DestroyTrigger(const TriggerID id);

private:
    // list of all registered triggers in this system
    Common::Map<TriggerID, TriggerObject> mTriggers;

    Common::BVH mTriggersTree;

    // list of triggers that overlapping pairs must be found for
    Common::HashSet<TriggerID> mInvalidatedTriggerIDs;

    // list of all overlapping trigger pairs
    Common::Set<TriggerInteraction> mInteractions;

    uint32 mTriggerID;

    void InvalidateArea(const Math::Box& box);
};

} // namespace Scene
} // namespace NFE
