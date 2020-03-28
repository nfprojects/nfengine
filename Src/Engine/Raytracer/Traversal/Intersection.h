#pragma once

#include "../../Common/Math/Matrix4.hpp"

namespace NFE {
namespace RT {

class Material;

// more detailed ray-shape intersection data
struct IntersectionData
{
    // geometry data
    Math::Matrix4 frame;
    Math::Vec4f texCoord;
    const Material* material = nullptr;
    const IMedium* medium = nullptr;

    NFE_FORCE_INLINE const Math::Vec4f LocalToWorld(const Math::Vec4f& localCoords) const
    {
        return frame.TransformVector(localCoords);
    }

    NFE_FORCE_INLINE const Math::Vec4f WorldToLocal(const Math::Vec4f& worldCoords) const
    {
        Math::Vec4f worldToLocalX = frame[0];
        Math::Vec4f worldToLocalY = frame[1];
        Math::Vec4f worldToLocalZ = frame[2];
        Math::Vec4f::Transpose3(worldToLocalX, worldToLocalY, worldToLocalZ);

        Math::Vec4f result = worldToLocalX * worldCoords.x;
        result = Math::Vec4f::MulAndAdd(worldToLocalY, worldCoords.y, result);
        result = Math::Vec4f::MulAndAdd(worldToLocalZ, worldCoords.z, result);
        return result;
    }

    NFE_FORCE_INLINE float CosTheta(const Math::Vec4f& dir) const
    {
        return Math::Vec4f::Dot3(frame[2], dir);
    }
};

} // namespace RT
} // namespace NFE
