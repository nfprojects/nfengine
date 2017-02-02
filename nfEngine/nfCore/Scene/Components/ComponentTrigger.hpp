/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of trigger component class.
 */

#pragma once

#include "Component.hpp"

#include "nfCommon/Math/Vector.hpp"


namespace NFE {
namespace Scene {

using TriggerID = uint32;

/**
 * There are two types of triggers: source and targets.
 * Source objects are triggering target ones (so two triggers of the same type cannot interact with themselves).
 * It's a kind of high-level filtering and optimization.
 */
enum class TriggerType : uint8
{
    Source, // e.g. player
    Target  // e.g. trigger volume
};

/**
 * Trigger Component.
 *
 * Registers in the trigger system and allows parent entity to receive
 * trigger events.
 */
class CORE_API NFE_ALIGN16 TriggerComponent : public IComponent
{
    NFE_DECLARE_POLYMORPHIC_CLASS(TriggerComponent)

public:
    TriggerComponent();
    ~TriggerComponent();

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;
    virtual void OnAttach() override;
    virtual void OnDetach() override;

    const Math::Vector& GetSize() const { return mSize; }
    TriggerType GetType() const { return mType; }

    /**
     * Set trigger box size.
     */
    void SetSize(const Math::Vector& newSize);

    /**
     * Change trigger type.
     */
    void SetType(TriggerType newType);

private:
    // TODO temporary
    // there must be a support for arbitrary shapes in the future
    Math::Vector mSize;

    // trigger ID (obtained from trigger system after registration)
    TriggerID mID;

    TriggerType mType;

    // TODO filtering (trigger include/exclude mask)
    // TODO CCD

    TriggerSystem* GetTriggerSystem() const;
};

} // namespace Scene
} // namespace NFE
