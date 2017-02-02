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


namespace NFE {
namespace Scene {

// TODO
// everything here is a proof-of-concept

using TriggerID = uint32;

struct NFE_ALIGN16 TriggerDesc
{
    Math::Box box;
    uint32 inculdeMask;
    uint32 excludeMask;

    TriggerDesc()
        : inculdeMask(0xFFFFFFFF)
        , excludeMask(0)
    { }
};


/**
 * Trigger system.
 */
class CORE_API NFE_ALIGN16 TriggerSystem final
    : public ISystem
{
public:
    TriggerSystem(SceneManager* scene)
        : ISystem(scene)
    { }


    TriggerID CreateTrigger(const TriggerDesc& desc);

    bool UpdateTrigger(const TriggerID id, const TriggerDesc& newDesc);

    void DestroyTrigger(const TriggerID id);

    void Update(float timeDelta) override;

private:
    // list of all registered triggers in this system
    Common::PackedArray<TriggerDesc, TriggerID, 16> mTriggers;
};

} // namespace Scene
} // namespace NFE
