/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of transform component.
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"
#include "nfCommon/Memory/Aligned.hpp"
#include "nfCommon/Math/Quaternion.hpp"

#include <set>


namespace NFE {
namespace Scene {

// TODO: consider moving to nfCommon/Math and renaming to Matrix3x4
class Orientation : public Common::Aligned<16>
{
public:
    Math::Vector x, y, z;

    Orientation()
    {
        x = Math::Vector(1.0f, 0.0f, 0.0f, 0.0f);
        y = Math::Vector(0.0f, 1.0f, 0.0f, 0.0f);
        z = Math::Vector(0.0f, 0.0f, 1.0f, 0.0f);
    }

    Orientation(const Math::Matrix& mat)
    {
        x = mat.r[0];
        y = mat.r[1];
        z = mat.r[2];
    }
};


#define NFE_TRANSFORM_FLAG_GLOBAL_MOVED (1 << 0)
#define NFE_TRANSFORM_FLAG_LOCAL_MOVED  (1 << 1)

NFE_ALIGN16
class CORE_API TransformComponent
    : public ComponentBase<TransformComponent>
    , public Common::Aligned<16>
{
    friend class PhysicsSystem;
    friend class SceneManager;
    friend class TransformSystem;

    /**
     * Global transform matrix (equals to mLocalMatrix * parent->mMatrix)
     */
    Math::Matrix mMatrix;
    Math::Matrix mLocalMatrix; //< Local transform matrix (relative to parent)
    int mFlags;

    /// TOOD: temporary - it's not cache-friendly solution
    TransformComponent* mParent;
    std::set<TransformComponent*> mChildren;

public:
    TransformComponent();

    void Invalidate();


    /**
     * Get global position and orientation as 4x4 matrix.
     */
    NFE_INLINE Math::Matrix GetMatrix() const
    {
        return mMatrix;
    }

    /**
    * Get local position and orientation as 4x4 matrix.
    */
    NFE_INLINE Math::Matrix GetLocalMatrix() const
    {
        return mLocalMatrix;
    }

    /**
     * Get global position in the scene.
     */
    NFE_INLINE Math::Vector GetPosition() const
    {
        return Math::Vector(mMatrix.r[3]) & Math::VECTOR_MASK_XYZ;
    }

    /**
     * Get position relative to parent entity.
     */
    NFE_INLINE Math::Vector GetLocalPosition() const
    {
        return Math::Vector(mLocalMatrix.r[3]) & Math::VECTOR_MASK_XYZ;
    }

    /**
     * Get global orientation.
     */
    Orientation GetOrientation() const;

    /**
     * Get current orientation relative to parent entity.
     */
    Orientation GetLocalOrientation() const;


    /**
     * Set global position and orientation as 4x4 matrix.
     */
    void SetMatrix(const Math::Matrix& matrix);

    /**
    * Set global position.
    */
    void SetPosition(const Math::Vector& pos);

    /**
     * Set global orientation using quaternion.
     */
    void SetOrientation(const Math::Quaternion& quat);

    /**
     * Set global orientation using rotation matrix.
     */
    void SetOrientation(const Orientation& orientation);


    /**
     * Set position relative to parent entity.
     */
    void SetLocalPosition(const Math::Vector& pos);

    /**
     * Set orientation relative to parent entity.
     */
    void SetLocalOrientation(const Orientation& orientation);
};

} // namespace Scene
} // namespace NFE
