/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declaration of transform component.
 */

#pragma once

#include "../Core.hpp"
#include "../Aligned.hpp"
#include "Component.hpp"
#include "../Aligned.hpp"

namespace NFE {
namespace Scene {

// TODO: consider moving to nfCommon/Math
class Orientation : public Util::Aligned
{
public:
    Math::Vector x, y, z;

    Orientation()
    {
        x = Math::Vector(1.0f, 0.0f, 0.0f, 0.0f);
        y = Math::Vector(0.0f, 1.0f, 0.0f, 0.0f);
        z = Math::Vector(0.0f, 0.0f, 1.0f, 0.0f);
    }

    Orientation(const Math::Matrix & mat)
    {
        x = mat.r[0];
        y = mat.r[1];
        z = mat.r[2];
    }
};

#define NFE_TRANSFORM_FLAG_GLOBAL_MOVED 1
#define NFE_TRANSFORM_FLAG_LOCAL_MOVED 2

NFE_ALIGN16
class CORE_API TransformComponent : public ComponentBase<TransformComponent>, public Util::Aligned
{
    Math::Matrix mMatrix;         //< global transform matrix (mMatrix = mLocalMatrix * parent->mMatrix)
    Math::Matrix mLocalMatrix;    //< local transform matrix (relative to parent)
    int mFlags;

    // TODO: parent and children

public:
    TransformComponent();

    /**
     * Set global position and orientation as 4x4 matrix.
     */
    void SetMatrix(const Math::Matrix& matrix);

    /**
     * Get global position and orientation as 4x4 matrix.
     */
    NFE_INLINE Math::Matrix GetMatrix() const
    {
        return mMatrix;
    }

    /**
    * Set global position.
    */
    void SetPosition(const Math::Vector& pos);

    /**
     * Get global position in the scene.
     */
    NFE_INLINE Math::Vector GetPosition() const
    {
        return mMatrix.r[3];
    }

    /**
     * Set global orientation using quaternion.
     */
    void SetOrientation(const Math::Quaternion& quat);

    /**
     * Set global orientation using rotation matrix.
     */
    void SetOrientation(const Orientation& orientation);

    /**
     * Get global orientation.
     */
    Orientation GetOrientation() const;


    /**
     * Set position relative to parent entity.
     */
    void SetLocalPosition(const Math::Vector& pos);

    /**
     * Get position relative to parent entity.
     */
    Math::Vector GetLocalPosition() const;

    /**
     * Set orientation relative to parent entity.
     */
    void SetLocalOrientation(const Orientation& orientation);

    /**
     * Get current orientation relative to parent entity.
     */
    Orientation GetLocalOrientation() const;
};

} // namespace Scene
} // namespace NFE
