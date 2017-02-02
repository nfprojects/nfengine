/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of physics system.
 */

#pragma once

#include "../../Core.hpp"
#include "System.hpp"
#include "../../Physics/PhysicsScene.hpp"

#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Math/Vector.hpp"
#include "nfCommon/Math/Matrix.hpp"


namespace NFE {
namespace Scene {


/**
 * Physics system interface.
 *
 * @note Extracting the interface from the actual implementation hides
 * underlying physics library (Bullet in this case), so there is no symbols pollution.
 * It could be also possible to implement other "physics backends" in the future.
 */
class PhysicsSystem final
    : public ISystem
{
    NFE_DECLARE_POLYMORPHIC_CLASS(PhysicsSystem)
    NFE_MAKE_NONCOPYABLE(PhysicsSystem);

public:
    static const int ID = 4;

    PhysicsSystem(SceneManager& scene);

    /**
     * Get physics scene.
     */
    Physics::IPhysicsScene* GetPhysicsScene() const { return mPhysicsScene.Get(); }

    void Update(const SystemUpdateContext& context) override;

private:
    Common::UniquePtr<Physics::IPhysicsScene> mPhysicsScene;
};


} // namespace Scene
} // namespace NFE
