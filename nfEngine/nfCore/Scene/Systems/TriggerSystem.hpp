/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of trigger system.
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"

#include "nfCommon/Containers/PackedArray.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Math/Box.hpp"
#include "nfCommon/Utils/BVH.hpp"


namespace NFE {
namespace Scene {

// TODO
// everything here is a proof-of-concept

using TriggerID = uint32;

struct NFE_ALIGN(16) TriggerDesc
{
    Math::Box box;
    bool isTarget;
    uint32 treeNodeId;
};

struct NFE_ALIGN(16) TriggerObject
{
    Math::Box box;

    // TODO this is a joke...
    std::unordered_set<TriggerID> interactions;

    uint32 treeNodeId;
};

/**
 * Trigger system.
 */
class CORE_API NFE_ALIGN(16) TriggerSystem final
    : public ISystem
    , public Common::Aligned<16>
{
public:
    static const int ID = 5;

    explicit TriggerSystem(SceneManager* scene)
        : ISystem(scene)
    { }

    ~TriggerSystem();

    /**
     * Update the trigger system.
     * @note    This will generate trigger events.
     */
    void Update(float timeDelta) override;

    /**
     * Create a new trigger object.
     * @notes   When trigger is created in overlapping state, it will generate "enter" event.
     */
    TriggerID CreateTrigger(const TriggerDesc& desc);

    /**
     * Update an existing trigger object.
     */
    void UpdateTrigger(const TriggerID id, const TriggerDesc& newDesc);

    /**
     * Destroy an existing trigger object.
     */
    void DestroyTrigger(const TriggerID id);

private:
    // list of all registered triggers in this system
    Common::PackedArray<TriggerObject, TriggerID, 16> mTriggers;

    Common::BVH mTriggersTree;

    // list of triggers that overlapping pairs must be found for
    std::unordered_set<TriggerID> mInvalidatedTriggers;

    void InvalidateArea(const Math::Box& box);
};

} // namespace Scene
} // namespace NFE
