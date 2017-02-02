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

class CORE_API TriggerComponent : public IComponent
{
public:
    TriggerComponent();
    ~TriggerComponent();

    // IComponent interface implementation
    virtual Math::Box GetBoundingBox() const override;
    virtual void OnAttach() override;
    virtual void OnDetach() override;

    /**
     * Get trigger box size.
     */
    const Math::Vector& GetSize() const { return mSize; }

    /**
     * Set trigger box size.
     */
    void SetSize(const Math::Vector& newSize);

private:
    // TODO temporary
    // there must be support for arbitrary shapes
    Math::Vector mSize;

    TriggerID mID;

    TriggerSystem* GetTriggerSystem() const;
};

} // namespace Scene
} // namespace NFE
