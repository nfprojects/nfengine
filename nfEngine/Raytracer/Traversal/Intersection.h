#pragma once

#include "../../nfCommon/Math/Matrix4.hpp"

namespace NFE {
namespace RT {

class Material;

// more detailed ray-shape intersection data
struct IntersectionData
{
    // geometry data
    Math::Matrix4 frame;
    Math::Vector4 texCoord;
    const Material* material = nullptr;

    NFE_FORCE_INLINE const Math::Vector4 LocalToWorld(const Math::Vector4& localCoords) const
    {
        return frame.TransformVector(localCoords);
    }

    NFE_FORCE_INLINE const Math::Vector4 WorldToLocal(const Math::Vector4& worldCoords) const
    {
        Math::Vector4 worldToLocalX = frame[0];
        Math::Vector4 worldToLocalY = frame[1];
        Math::Vector4 worldToLocalZ = frame[2];
        Math::Vector4::Transpose3(worldToLocalX, worldToLocalY, worldToLocalZ);

        Math::Vector4 result = worldToLocalX * worldCoords.x;
        result = Math::Vector4::MulAndAdd(worldToLocalY, worldCoords.y, result);
        result = Math::Vector4::MulAndAdd(worldToLocalZ, worldCoords.z, result);
        return result;
    }

    NFE_FORCE_INLINE float CosTheta(const Math::Vector4& dir) const
    {
        return Math::Vector4::Dot3(frame[2], dir);
    }
};

} // namespace RT
} // namespace NFE
